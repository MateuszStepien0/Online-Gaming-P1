#include "Server.h"
#include "PacketStructs.h"
#include "Game.h"
#pragma comment(lib,"ws2_32.lib")

Server::Server(int port)
{
	WSAData wsa;
	WORD DllVersion = MAKEWORD(2, 1);

	if (WSAStartup(DllVersion, &wsa) != 0)
	{
		MessageBoxA(NULL, "Winsock startup failed", "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	//addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	listener = socket(AF_INET, SOCK_STREAM, NULL);

	if (bind(listener, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		std::string ErrorMsg = "Failed to bind the address to our listening socket. Winsock Error:" + std::to_string(WSAGetLastError());
		MessageBoxA(NULL, ErrorMsg.c_str(), "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	if (listen(listener, SOMAXCONN) == SOCKET_ERROR)
	{
		std::string ErrorMsg = "Failed to listen on listening socket. Winsock Error:" + std::to_string(WSAGetLastError());
		MessageBoxA(NULL, ErrorMsg.c_str(), "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	IDCounter = 0;

	std::thread PST(packetSenderThread, std::ref(*this));
	PST.detach();
	threads.push_back(&PST);
}

Server::~Server()
{
	terminateThreads = true;

	for (std::thread* t : threads)
	{
		t->join();
	}
}

bool Server::listenForConnection()
{
	if (activeConnections == 2)
	{
		return false;
	}

	int addrlen = sizeof(addr);
	SOCKET newConnection = accept(listener, (SOCKADDR*)&addr, &addrlen);

	if (newConnection == 0)
	{
		std::cout << "Failed to accept client's connection.\n";
		return false;
	}
	else
	{
		std::lock_guard<std::shared_mutex> lock(mutex_connectionMgr);
		std::shared_ptr<Connection> newConnection(std::make_shared<Connection>(newConnection));
		connections.push_back(newConnection);

		newConnection->ID = IDCounter;
		IDCounter += 1;
		activeConnections += 1;

		std::cout << "Client Connected! ID:" << newConnection->ID << std::endl;

		std::thread CHT(clientHandlerThread, std::ref(*this), newConnection);
		CHT.detach();
		threads.push_back(&CHT);

		return true;
	}
}

bool Server::getint32t(std::shared_ptr<Connection> connection, std::int32_t& int32t)
{
	if (!receiveAll(connection, (char*)&int32t, sizeof(std::int32_t)))
	{
		return false;
	}

	int32t = ntohl(int32t);
	return true;
}

bool Server::getPacketType(std::shared_ptr<Connection> connection, PacketType& packetType)
{
	std::int32_t packettype_int;

	if (!getint32t(connection, packettype_int))
	{
		return false;
	}

	packetType = (PacketType)packettype_int;
	return true;
}

bool Server::getString(std::shared_ptr<Connection> connection, std::string& string)
{
	std::int32_t bufferlength;

	if (!getint32t(connection, bufferlength))
	{
		return false;
	}

	if (bufferlength == 0)
	{
		return true;
	}

	string.resize(bufferlength);
	return receiveAll(connection, &string[0], bufferlength);
}

bool Server::getPlayerUpdate(std::shared_ptr<Connection> connection, PlayerData& updateData)
{
	std::int32_t bufferlength;

	if (!getint32t(connection, bufferlength))
	{
		return false;
	}

	if (bufferlength == 0)
	{
		return true;
	}

	return receiveAll(connection, (char*)&updateData, bufferlength);
}

bool Server::getGameStart(std::shared_ptr<Connection> connection, StartData& startData)
{
	std::int32_t bufferlength;

	if (!getint32t(connection, bufferlength))
	{
		return false;
	}

	if (bufferlength == 0)
	{
		return true;
	}

	return receiveAll(connection, (char*)&startData, bufferlength);
}

void Server::sendString(const std::string& string)
{
	PS::ChatMessage message(string);

	for (auto connection : connections)
	{
		connection->pm.Append(message.toPacket());
	}
}

void Server::sendPlayerUpdate(PlayerData& updateData)
{
	PS::PlayerUpdate gameUpdate(updateData);
	std::shared_ptr<Packet> packet = std::make_shared<Packet>(gameUpdate.toPacket());
	std::shared_lock<std::shared_mutex> lock(mutex_connectionMgr);

	for (auto connection : connections)
	{
		connection->pm.Append(packet);
	}
}

void Server::sendGameStart(StartData& startData, int index)
{
	PS::GameStart gameStart(startData);
	std::shared_ptr<Packet> packet = std::make_shared<Packet>(gameStart.toPacket());
	std::shared_lock<std::shared_mutex> lock(mutex_connectionMgr);

	connections[index]->pm.Append(packet);
}

void Server::sendGameEnd(EndData& endData)
{
	PS::GameEnd gameEnd(endData);
	std::shared_ptr<Packet> packet = std::make_shared<Packet>(gameEnd.toPacket());
	std::shared_lock<std::shared_mutex> lock(mutex_connectionMgr);

	for (auto connection : connections)
	{
		connection->pm.Append(packet);
	}
}

void Server::sendGameUpdate(std::array<sf::Vector2f, 3>& positions)
{
	PS::GameUpdate gameUpdate(positions);
	std::shared_ptr<Packet> packet = std::make_shared<Packet>(gameUpdate.toPacket());
	std::shared_lock<std::shared_mutex> lock(mutex_connectionMgr);

	for (auto connection : connections)
	{
		connection->pm.Append(packet);
	}
}

void Server::sendChangeState(const GameState& gameState)
{
	PS::StateChange stateChange(gameState);
	std::shared_ptr<Packet> packet = std::make_shared<Packet>(stateChange.toPacket());
	std::shared_lock<std::shared_mutex> lock(mutex_connectionMgr);

	for (auto connection : connections)
	{
		connection->pm.Append(packet);
	}
}

void Server::disconnectClient(std::shared_ptr<Connection> connection)
{
	std::lock_guard<std::shared_mutex> lock(mutex_connectionMgr);

	connection->pm.Clear();
	closesocket(connection->socket);
	connections.erase(std::remove(connections.begin(), connections.end(), connection));

	std::cout << "Cleaned up client: " << connection->ID << "." << std::endl;
	std::cout << "Total connections: " << connections.size() << std::endl;

	activeConnections -= 1;
}

bool Server::sendAll(std::shared_ptr<Connection> connection, const char* data, const int totalBytes)
{
	int bytesSent = 0;

	while (bytesSent < totalBytes)
	{
		int returnCheck = send(connection->socket, data + bytesSent, totalBytes - bytesSent, 0);

		if (returnCheck == SOCKET_ERROR)
		{
			return false;
		}
		bytesSent += returnCheck;
	}

	return true;
}

bool Server::receiveAll(std::shared_ptr<Connection> connection, char* data, int totalBytes)
{
	int bytesReceived = 0;

	while (bytesReceived < totalBytes)
	{
		int returnCheck = recv(connection->socket, data + bytesReceived, totalBytes - bytesReceived, 0);

		if (returnCheck == SOCKET_ERROR || returnCheck == 0) 
		{
			return false;
		}
		bytesReceived += returnCheck;
	}

	return true;
}

bool Server::processPacket(std::shared_ptr<Connection> connection, PacketType packetType)
{
	switch (packetType)
	{
	case PacketType::MESSAGE:
	{
		std::string message;

		if (!getString(connection, message))
		{
			return false;
		}

		PS::ChatMessage cm(message);
		std::shared_ptr<Packet> msgPacket = std::make_shared<Packet>(cm.toPacket());
		{
			std::shared_lock<std::shared_mutex> lock(mutex_connectionMgr);
			for (auto conn : connections)
			{
				if (conn == connection)
				{
					continue;
				}
				conn->pm.Append(msgPacket);
			}
		}

		std::cout << "Processed chat message packet from user ID: " << connection->ID << std::endl;
		break;
	}
	case PacketType::PLAYERREADY:
	{
		StartData data;

		if (!getGameStart(connection, data))
		{
			return false;
		}

		break;
	}
	case PacketType::PLAYERUPDATE:
	{
		PlayerData data;

		if (!getPlayerUpdate(connection, data))
		{
			return false;
		}

		game->updatePlayer(data);
		break;
	}
	default:
	{
		std::cout << "Unrecognized packet: " << (std::int32_t)packetType << std::endl;
		return false;
	}
	}
	return true;
}

void Server::clientHandlerThread(Server& server, std::shared_ptr<Connection> connection)
{
	PacketType packettype;
	while (true)
	{
		if (server.terminateThreads == true)
		{
			break;
		}
		if (!server.getPacketType(connection, packettype))
		{
			break;
		}
		if (!server.processPacket(connection, packettype))
		{
			break;
		}
	}

	std::cout << "Lost connection to client ID: " << connection->ID << std::endl;
	server.disconnectClient(connection);
	return;
}

void Server::packetSenderThread(Server& server)
{
	while (true)
	{
		if (server.terminateThreads == true)
		{
			break;
		}
		std::shared_lock<std::shared_mutex> lock(server.mutex_connectionMgr);
		for (auto conn : server.connections)
		{
			if (conn->pm.HasPendingPackets())
			{
				std::shared_ptr<Packet> p = conn->pm.Retrieve();
				if (!server.sendAll(conn, (const char*)(&p->m_buffer[0]), p->m_buffer.size()))
				{
					std::cout << "Failed to send packet to ID: " << conn->ID << std::endl;
				}
				std::cout << "Sent packet to ID: " << conn->ID << std::endl;
			}
		}
		Sleep(5);
	}

	std::cout << "Ending Packet Sender Thread..." << std::endl;
}
