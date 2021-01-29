#include "Client.h"
#include "PacketStructs.h"
#include "Game.h"
#include <Ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

Client::Client(const char* ip, const int port)
{
	//Winsock Startup
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0)
	{
		MessageBoxA(0, "Winsock startup failed", "Error", MB_OK | MB_ICONERROR);
		exit(0);
	}

	inet_pton(AF_INET, ip, &addr.sin_addr.s_addr);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
}

Client::~Client()
{
	closeConnection();
	sendThread.join();
	listenThread.join();
}

bool Client::Connect()
{
	connection = socket(AF_INET, SOCK_STREAM, 0);
	int addrlen = sizeof(addr);

	if (connect(connection, (SOCKADDR*)&addr, addrlen) != 0)
	{
		MessageBoxA(0, "Failed to Connect", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	std::cout << "Connected!" << std::endl;

	sendThread = std::thread(packetSenderThread, std::ref(*this));
	sendThread.detach();

	listenThread = std::thread(clientThread, std::ref(*this));
	listenThread.detach();

	return true;
}

void Client::Disconnect()
{
	packetManager.Clear();
	closesocket(connection);
	std::cout << "Disconnected from server." << std::endl;
}

void Client::sendPlayerUpdate(const PlayerData& updateData)
{
	PS::PlayerUpdate gameStart(updateData);
	packetManager.Append(gameStart.toPacket());
}

void Client::sendGameStart(const StartData& startData)
{
	PS::GameStart gameStart(startData);
	packetManager.Append(gameStart.toPacket());
}

bool Client::closeConnection()
{
	terminateThreads = true;

	if (closesocket(connection) == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAENOTSOCK)
		{
			return true;
		}

		std::string ErrorMessage = "Failed to close the socket. Winsock Error: " + std::to_string(WSAGetLastError()) + ".";
		MessageBoxA(0, ErrorMessage.c_str(), "Error", MB_OK | MB_ICONERROR);

		return false;
	}

	return true;
}

bool Client::processPacketType(const PacketType packetType)
{
	std::cout << "Packet type: " << (int)packetType << std::endl;
	switch (packetType)
	{
	case PacketType::MESSAGE:
	{
		std::string message;
		if (!getString(message))
		{
			return false;
		}
		std::cout << message << std::endl;
		break;
	}
	case PacketType::PLAYERREADY:
	{
		StartData data;
		if (!getGameStart(data))
		{
			return false;
		}
		game->setStartData(data);
		break;
	}
	case PacketType::PLAYERUPDATE:
	{
		break;
	}
	case PacketType::GAMEUPDATE:
	{
		std::array<sf::Vector2f, 3> positions;
		if (!getUpdateGame(positions))
		{
			return false;
		}
		game->updatePositions(positions);
		break;
	}
	case PacketType::GAMEEND:
	{
		EndData data;
		if (!getGameEnd(data))
		{
			return false;
		}
		game->setEndData(data);
		break;
	}
	case PacketType::STATECHANGE:
	{
		GameState changeState;
		if (!getChangeState(changeState))
		{
			return false;
		}
		game->changeState(changeState);
		break;
	}
	default:

		std::cout << "Unrecognized PacketType: " << (std::int32_t)packetType << std::endl;
		break;
	}

	return true;
}

void Client::clientThread(Client& client)
{
	PacketType PacketType;
	while (true)
	{
		if (client.terminateThreads == true)
		{
			break;
		}
		if (!client.getPacketType(PacketType))
		{
			break;
		}
		if (!client.processPacketType(PacketType))
		{
			break;
		}
	}

	std::cout << "Lost connection to the server.\n";
	client.terminateThreads = true;

	if (client.closeConnection())
	{
		std::cout << "Socket to the server was closed successfully." << std::endl;
	}
	else
	{
		std::cout << "Socket was not able to be closed." << std::endl;
	}
}

void Client::packetSenderThread(Client& client)
{
	while (true)
	{
		if (client.terminateThreads == true)
		{
			break;
		}

		while (client.packetManager.HasPendingPackets())
		{
			std::shared_ptr<Packet> p = client.packetManager.Retrieve();

			if (!client.sendAll((const char*)(&p->m_buffer[0]), p->m_buffer.size()))
			{
				std::cout << "Failed to send packet to server..." << std::endl;
				break;
			}
		}
		Sleep(5);
	}
	std::cout << "Packet thread closing..." << std::endl;
}

bool Client::sendAll(const char* data, int totalBytes)
{
	int bytesSent = 0;

	while (bytesSent < totalBytes)
	{
		int returnCheck = send(connection, data + bytesSent, totalBytes - bytesSent, 0);

		if (returnCheck == SOCKET_ERROR)
		{
			return false;
		}
		bytesSent += returnCheck;
	}

	return true;
}

bool Client::receiveAll(char* data, int totalBytes)
{
	int bytesReceived = 0;

	while (bytesReceived < totalBytes)
	{
		int returnCheck = recv(connection, data + bytesReceived, totalBytes - bytesReceived, 0);

		if (returnCheck == SOCKET_ERROR)
		{
			return false;
		}
		bytesReceived += returnCheck;
	}
	return true;
}

bool Client::getInt32_t(std::int32_t& int32_t)
{
	if (!receiveAll((char*)&int32_t, sizeof(std::int32_t)))
	{
		return false;
	}
	int32_t = ntohl(int32_t);

	return true;
}

bool Client::getPacketType(PacketType& packetType)
{
	std::int32_t packetType_int;

	if (!getInt32_t(packetType_int))
	{
		return false;
	}
	packetType = (PacketType)packetType_int;

	return true;
}

bool Client::getString(std::string& string)
{
	std::int32_t bufferlength;

	if (!getInt32_t(bufferlength))
	{
		return false;
	}

	if (bufferlength == 0)
	{
		return true;
	}
	string.resize(bufferlength);

	return receiveAll(&string[0], bufferlength);
}

bool Client::getPlayerUpdate(PlayerData& updateData)
{
	std::int32_t bufferlength;

	if (!getInt32_t(bufferlength))
	{
		return false;
	}

	if (bufferlength == 0)
	{
		return true;
	}

	return receiveAll((char*)&updateData, bufferlength);
}

bool Client::getGameStart(StartData& startData)
{
	std::int32_t bufferlength;

	if (!getInt32_t(bufferlength))
	{
		return false;
	}

	if (bufferlength == 0)
	{
		return true;
	}

	return receiveAll((char*)&startData, bufferlength);
}

bool Client::getGameEnd(EndData& endData)
{
	std::int32_t bufferlength;

	if (!getInt32_t(bufferlength))
	{
		return false;
	}

	if (bufferlength == 0)
	{
		return true;
	}

	return receiveAll((char*)&endData, bufferlength);
}

bool Client::getChangeState(GameState& changeState)
{
	std::int32_t bufferlength;

	if (!getInt32_t(bufferlength))
	{
		return false;
	}

	if (bufferlength == 0)
	{
		return true;
	}

	return receiveAll((char*)&changeState, bufferlength);
}

bool Client::getUpdateGame(std::array<sf::Vector2f, 3>& positions)
{
	std::int32_t bufferlength;

	if (!getInt32_t(bufferlength))
	{
		return false;
	}

	if (bufferlength == 0)
	{
		return true;
	}

	return receiveAll((char*)&positions[0], bufferlength);
}
