#include "Game.h"

static double const MS_PER_UPDATE = 10.0;

Game::Game() : 
	state(GameState::SELECT),
	window(sf::VideoMode(900, 900, 32), "Online Gaming P1")
{
	window.setVerticalSyncEnabled(true);
	srand(std::time(NULL));

	if (!font.loadFromFile("Assets/Fonts/arcade.ttf"))
	{
		std::cout << "font not loaded\n";
	}

	player[0].setName("Red");
	player[0].setStartPosition(sf::Vector2f(150, 150));
	player[0].setPosition(player[0].getStartPosition());
	player[0].setColor(sf::Color::Red);

	player[1].setName("Green");
	player[1].setStartPosition(sf::Vector2f(450, 150));
	player[1].setPosition(player[1].getStartPosition());
	player[1].setColor(sf::Color::Green);

	player[2].setName("Blue");
	player[2].setStartPosition(sf::Vector2f(150, 450));
	player[2].setPosition(player[2].getStartPosition());
	player[2].setColor(sf::Color::Blue);
	
	hostBut.setFillColor(sf::Color::Cyan);
	hostBut.setOutlineThickness(3);
	hostBut.setOutlineColor(sf::Color::Black);
	hostBut.setSize(sf::Vector2f(400, 100));
	hostBut.setOrigin(sf::Vector2f(200, 50));
	hostBut.setPosition(450, 500);

	hostText.setFont(font);
	hostText.setFillColor(sf::Color::Black);
	hostText.setCharacterSize(50);
	hostText.setString("Host");
	hostText.setPosition(sf::Vector2f(400, 466));

	joinBut.setFillColor(sf::Color::Cyan);
	joinBut.setOutlineThickness(3);
	joinBut.setOutlineColor(sf::Color::Black);
	joinBut.setSize(sf::Vector2f(400, 100));
	joinBut.setOrigin(sf::Vector2f(200, 50));
	joinBut.setPosition(450, 700);

	joinText.setFont(font);
	joinText.setFillColor(sf::Color::Black);
	joinText.setCharacterSize(50);
	joinText.setString("Join");
	joinText.setPosition(sf::Vector2f(400, 666));

	waitingText.setFont(font);
	waitingText.setFillColor(sf::Color::Black);
	waitingText.setCharacterSize(45);
	waitingText.setString("Waiting   for   players");
	waitingText.setPosition(sf::Vector2f(100, 100));

	startingText.setFont(font);
	startingText.setFillColor(sf::Color::Black);
	startingText.setCharacterSize(45);
	startingText.setString("");
	startingText.setPosition(sf::Vector2f(100, 100));

	endingText.setFont(font);
	endingText.setFillColor(sf::Color::Black);
	endingText.setCharacterSize(45);
	endingText.setString("");
	endingText.setPosition(sf::Vector2f(100, 100));

	timeText.setFont(font);
	timeText.setFillColor(sf::Color::Black);
	timeText.setCharacterSize(45);
	timeText.setString("");
	timeText.setPosition(sf::Vector2f(100, 175));
}

void Game::run()
{
	sf::Clock clock;
	sf::Time timeSinceLastUpdate = sf::Time::Zero;
	sf::Time timePerFrame = sf::seconds(1.f / 60.f);

	while (window.isOpen())
	{
		processEvents();
		timeSinceLastUpdate += clock.restart();
		while (timeSinceLastUpdate > timePerFrame)
		{
			timeSinceLastUpdate -= timePerFrame;
			processEvents();
			update(timePerFrame);
		}
		render();
	}
}

void Game::updatePlayer(PlayerData data)
{
	player[data.index].updatePosition(data.position);
}

void Game::updatePositions(std::array<sf::Vector2f, 3> positions)
{
	for (int i = 0; i < 3; i++)
	{
		player[i].setPosition(positions[i]);
	}
}

void Game::setStartData(StartData data)
{
	changeState(GameState::START);

	targetIndex = data.targetIndex;
	playerIndex = data.index;

	std::string message;

	message = "You   are   " + player[playerIndex].getName();

	if (targetIndex == playerIndex)
	{
		message += "   Try   to   run";
	}
	else
	{
		message += "   Try   to   catch " + player[targetIndex].getName();
	}

	startingText.setString(message);
}

void Game::setEndData(EndData data)
{
	changeState(GameState::GAMEOVER);

	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << data.time.asSeconds();
	std::string string = stream.str();

	if (data.index == playerIndex)
	{
		endingText.setString("You   caught   " + player[data.targetIndex].getName());
		timeText.setString(player[data.targetIndex].getName() + " " + string + "s");
	}
	else if (data.targetIndex == playerIndex)
	{
		endingText.setString(player[data.index].getName() + "   has   caught   you");
		timeText.setString("You   Lasted   " + string + "s");
	}
	else
	{
		endingText.setString(player[data.index].getName() + "   has   caught   " + player[data.targetIndex].getName());
		timeText.setString(player[data.targetIndex].getName() + " " + string + "s");
	}
}

void Game::changeState(GameState changeState)
{
	state = changeState;

	if (state == GameState::WAITING)
	{
		reset();
	}
}

void Game::update(sf::Time dt)
{
	if (state == GameState::WAITING)
	{
		if (host)
		{
			if (server->activeConnections == 2)
			{
				countdown = sf::seconds(5.0f);
				state = GameState::START;

				int pi = rand() % 3;
				int ti = rand() % 3;

				playerIndex = pi;
				targetIndex = ti;

				std::string message;

				message = "You   are " + player[playerIndex].getName();

				if (targetIndex == playerIndex)
				{
					message += "   Try   to   run";
				}
				else
				{
					message += "   Try   to   catch " + player[targetIndex].getName() + ".";
				}

				startingText.setString(message);

				StartData data;
				data.targetIndex = ti;

				for (int i = 0; i < 2; i++)
				{
					pi = (pi + 1) % 3;
					data.index = pi;
					server->sendGameStart(data, i);
				}
			}
		}
	}
	else if (state == GameState::START)
	{
		if (host)
		{
			countdown -= dt;

			if (countdown <= sf::seconds(0.0f))
			{
				state = GameState::GAMEPLAY;
				server->sendChangeState(GameState::GAMEPLAY);
			}
		}
	}
	else if (state == GameState::GAMEPLAY)
	{
		sf::Vector2f vec = player[playerIndex].movement();

		if (host)
		{
			if (server->activeConnections < 1)
			{
				changeState(GameState::WAITING);
				server->sendChangeState(GameState::WAITING);
				return;
			}

			player[playerIndex].updatePosition(vec);
			timeLasted += dt;

			if (!checkForCollisions())
			{
				std::array<sf::Vector2f, 3> positions;

				for (int i = 0; i < 3; i++)
				{
					positions[i] = player[i].getPosition();
				}

				server->sendGameUpdate(positions);
			}
		}
		else
		{
			PlayerData data;
			data.index = playerIndex;
			data.position = vec;
			client->sendPlayerUpdate(data);
		}
	}
}

void Game::render()
{
	window.clear(sf::Color::White);

	switch (state)
	{
	case GameState::SELECT:
		window.draw(hostBut);
		window.draw(joinBut);
		window.draw(hostText);
		window.draw(joinText);
		break;
	case GameState::WAITING:
		window.draw(waitingText);
		break;
	case GameState::START:
		window.draw(startingText);
		break;
	case GameState::GAMEPLAY:
		for (int i = 0; i < 3; i++)
		{
			player[i].draw(window);
		}
		break;
	case GameState::GAMEOVER:
		window.draw(endingText);
		window.draw(timeText);
		break;
	default:
		break;
	}

	window.display();
}

void Game::processEvents()
{
	sf::Event event;

	while (window.pollEvent(event))
	{
		sf::Vector2f pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

		if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
		{
			window.close();
		}

		if (state == GameState::SELECT)
		{
			if (event.type == sf::Event::MouseButtonPressed)
			{
				if (event.key.code == sf::Mouse::Left)
				{
					if (hostBut.getGlobalBounds().contains(pos.x, pos.y))
					{
						startServer();
						state = GameState::WAITING;
					}
					else if (joinBut.getGlobalBounds().contains(pos.x, pos.y))
					{
						connect();
						state = GameState::WAITING;
					}
				}
			}
		}
	}
}

bool Game::checkForCollisions()
{
	for (int i = 0; i < 3; i++)
	{
		if (i != targetIndex)
		{
			float dis = getDistance(player[targetIndex].getPosition(), player[i].getPosition());

			if (dis <= 30)
			{
				EndData data;
				data.index = i;
				data.targetIndex = targetIndex;
				data.time = timeLasted;

				server->sendGameEnd(data);
				setEndData(data);
				return true;
			}
		}
	}
	return false;
}

void Game::reset()
{
	timeLasted = sf::seconds(0.0f);

	for (int i = 0; i < 3; i++)
	{
		player[i].setPosition(player[i].getStartPosition());
	}
}

void Game::startServer()
{
	host = true;
	server = new Server(1111);
	server->game = this;

	std::thread servThread(&Game::listenForConnections, this);
	servThread.detach();

	serverThread = &servThread;
}

void Game::connect()
{
	client = new Client("127.0.0.1", 1111);
	client->game = this;

	if (!client->Connect())
	{
		client->Disconnect();
		window.close();
	}
}

void Game::listenForConnections()
{
	while (server->activeConnections < 5)
	{
		server->listenForConnection();
	}
}

float Game::getDistance(sf::Vector2f t_pos1, sf::Vector2f t_pos2)
{
	return sqrt(powf(t_pos1.x - t_pos2.x, 2) + powf(t_pos1.y - t_pos2.y, 2));
}
