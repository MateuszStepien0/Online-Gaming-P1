#pragma once
#include "Enums.h"
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <vector>
#include <memory>
#include <array>

struct PlayerData
{
	int index;
	sf::Vector2f position;
};

struct StartData
{
	int index;
	int targetIndex;
};

struct EndData
{
	int index;
	int targetIndex;
	sf::Time time;
};

class Packet
{
public:
	Packet();
	Packet(const char* buffer, const int size);
	Packet(const PacketType pt);
	Packet(const std::shared_ptr<Packet> p);

	void Append(const std::shared_ptr<Packet> p);
	void Append(const PacketType p);
	void Append(const std::int32_t int32);
	void Append(const std::size_t p);
	void Append(const Packet & p);
	void Append(const std::string & str);
	void Append(const char * buffer, const int size);
	void Append(const PlayerData& updateData);
	void Append(const GameState& state);
	void Append(const StartData& startData);
	void Append(const EndData& endData);
	void Append(const std::array<sf::Vector2f, 3> positions);
	std::vector<int8_t> m_buffer; //Packet Buffer	
};