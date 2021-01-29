#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define NOMINMAX
#include <WinSock2.h> 
#include <string>
#include <iostream>
#include <vector>
#include <shared_mutex>
#include "PacketManager.h"

class Game;

class Connection
{
public:
	Connection(SOCKET socket)
		:socket(socket)
	{
	}
	SOCKET socket;
	PacketManager pm;
	int ID = 0;
};

class Server
{
public:
	Server(int port);
	~Server();

	Game* game;

	int IDCounter = 0;
	int activeConnections = 0;

	bool listenForConnection();

	bool getint32t(std::shared_ptr<Connection> connection, std::int32_t& int32t);
	bool getPacketType(std::shared_ptr<Connection> connection, PacketType& packetType);
	bool getString(std::shared_ptr<Connection> connection, std::string& string);
	bool getPlayerUpdate(std::shared_ptr<Connection> connection, PlayerData& updateData);
	bool getGameStart(std::shared_ptr<Connection> connection, StartData& startData);

	void sendString(const std::string& string);
	void sendPlayerUpdate(PlayerData& updateData);
	void sendGameStart(StartData& startData, int index);
	void sendGameEnd(EndData& endData);
	void sendGameUpdate(std::array<sf::Vector2f, 3>& positions);
	void sendChangeState(const GameState& gameState);

private:

	void disconnectClient(std::shared_ptr<Connection> connection);
	bool sendAll(std::shared_ptr<Connection> connection, const char* data, const int totalBytes);
	bool receiveAll(std::shared_ptr<Connection> connection, char* data, int totalBytes);
	bool processPacket(std::shared_ptr<Connection> connection, PacketType packetType);
	static void clientHandlerThread(Server& server, std::shared_ptr<Connection> connection);
	static void packetSenderThread(Server& server);

	SOCKADDR_IN addr;
	SOCKET listener;

	std::vector<std::shared_ptr<Connection>> connections;
	std::shared_mutex mutex_connectionMgr;

	bool terminateThreads = false;
	std::vector<std::thread*> threads;
};

