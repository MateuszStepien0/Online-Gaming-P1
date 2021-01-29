#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "Player.h"
#include "Server.h"
#include "Client.h"

class Game
{
public:
	Game();

	void run();
	void updatePlayer(PlayerData data);
	void updatePositions(std::array<sf::Vector2f, 3> positions);
	void setStartData(StartData data);
	void setEndData(EndData data);
	void changeState(GameState state);

private:
	void update(sf::Time dt);
	void render();
	void processEvents();
	bool checkForCollisions();
	void reset();
	void startServer();
	void connect();
	void listenForConnections();
	float getDistance(sf::Vector2f pos1, sf::Vector2f pos2);

	sf::RenderWindow window;

	Player player[3];
	GameState state;

	sf::Time timeLasted;
	sf::Time countdown;

	Server* server{ nullptr };
	Client* client{ nullptr };

	std::thread* serverThread;

	int playerIndex{ -1 };
	int targetIndex{ -1 };

	bool host{ false };

	sf::Font font;

	sf::RectangleShape hostBut;
	sf::Text hostText;
	sf::RectangleShape joinBut;
	sf::Text joinText;

	sf::Text waitingText;
	sf::Text startingText;
	sf::Text endingText;
	sf::Text timeText;

};

