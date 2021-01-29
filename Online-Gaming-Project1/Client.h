#pragma once
#include <SFML/Graphics.hpp>
#include <WinSock2.h>
#include <string>
#include <iostream>
#include "PacketManager.h"

class Game;

class Client
{
public:
	//const char* ip, int port
	Client(const char* ip, const int port);
	~Client();

	Game* game;

	bool Connect();
	void Disconnect();
	void sendPlayerUpdate(const PlayerData& updateData);
	void sendGameStart(const StartData& startData);
private:

	bool closeConnection();
	bool processPacketType(const PacketType packetType);
	static void clientThread(Client& client);
	static void packetSenderThread(Client& client);

	bool sendAll(const char* data, int totalBytes);
	bool receiveAll(char* data, int totalBytes);

	bool getInt32_t(std::int32_t& int32_t);
	bool getPacketType(PacketType& packetType);
	bool getString(std::string& string);
	bool getPlayerUpdate(PlayerData& updateData);
	bool getGameStart(StartData& startData);
	bool getGameEnd(EndData& endData);
	bool getChangeState(GameState& changeState);
	bool getUpdateGame(std::array<sf::Vector2f, 3>& positions);

	SOCKET connection;
	SOCKADDR_IN addr;
	PacketManager packetManager;

	std::thread sendThread;
	std::thread listenThread;

	bool terminateThreads = false;
	bool isConnected = false;
};

