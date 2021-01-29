#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>

class Player
{
public:
	Player();

	void update(double dt);
	void setName(std::string n);
	void setStartPosition(sf::Vector2f position);
	void updatePosition(sf::Vector2f velocity);
	void setPosition(sf::Vector2f newPosition);
	void setColor(sf::Color color);

	void draw(sf::RenderWindow& window);

	sf::Vector2f getPosition();
	sf::Vector2f movement();
	sf::Vector2f getStartPosition();
	std::string	getName();

private:
	sf::Vector2f position;
	sf::Vector2f startPosition;

	sf::CircleShape circle;
	std::string name;

	void boundaryCheck();

};