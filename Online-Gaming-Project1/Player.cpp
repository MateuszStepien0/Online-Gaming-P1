#include "Player.h"

Player::Player()
{
	position = sf::Vector2f(200, 200);

	circle.setRadius(15);
	circle.setOutlineColor(sf::Color::Black);
	circle.setOutlineThickness(3);
	circle.setFillColor(sf::Color::Red);
	circle.setOrigin(15, 15);

	circle.setPosition(position);
}

void Player::update(double dt)
{
	circle.setPosition(position);
	boundaryCheck();
}

void Player::draw(sf::RenderWindow& window)
{
	window.draw(circle);
}

void Player::setName(std::string n)
{
	name = n;
}

void Player::setStartPosition(sf::Vector2f position)
{
	startPosition = position;
}

void Player::updatePosition(sf::Vector2f velocity)
{
	position += velocity;
	circle.setPosition(position);
	boundaryCheck();
}

void Player::setPosition(sf::Vector2f newPosition)
{
	position = newPosition;
	circle.setPosition(position);
}

void Player::setColor(sf::Color color)
{
	circle.setFillColor(color);
}

sf::Vector2f Player::getPosition()
{
	return position;
}

sf::Vector2f Player::movement()
{
	sf::Vector2f vec = sf::Vector2f(0.0f, 0.0f);

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		vec = sf::Vector2f(0.0f, -2.0f);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		vec = sf::Vector2f(-2.0f, 0.0f);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		vec = sf::Vector2f(0.0f, 2.0f);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		vec = sf::Vector2f(2.0f, 0.0f);
	}

	return vec;
}

sf::Vector2f Player::getStartPosition()
{
	return startPosition;
}

std::string Player::getName()
{
	return name;
}

void Player::boundaryCheck()
{
	if (position.x < -15)
	{
		position.x = 915;
	}
	if (position.x > 915)
	{
		position.x = -15;
	}
	if (position.y < -15)
	{
		position.y = 915;
	}
	if (position.y > 915)
	{
		position.y = -15;
	}
}
