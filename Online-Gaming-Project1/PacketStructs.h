#pragma once
#include "Packet.h"
#include <array>
#include <string> 
#include <memory>

namespace PS //Packet Structures Namespace
{
	class ChatMessage
	{
	public:
		ChatMessage(const std::string & str);
		std::shared_ptr<Packet> toPacket(); //Converts ChatMessage to packet

	private:
		std::string m_message;
	};

	class StateChange
	{
	public:
		StateChange(const GameState& state);
		std::shared_ptr<Packet> toPacket();

	private:
		GameState state;
	};

	class GameStart
	{
	public:
		GameStart(const StartData& data);
		std::shared_ptr<Packet> toPacket();

	private:
		StartData data;
	};

	class PlayerUpdate
	{
	public:
		PlayerUpdate(const PlayerData& data);
		std::shared_ptr<Packet> toPacket();

	private:
		PlayerData data;
	};

	class GameUpdate
	{
	public:
		GameUpdate(const std::array<sf::Vector2f, 3>& positions);
		std::shared_ptr<Packet> toPacket();
	private:

		std::array<sf::Vector2f, 3> positions;
	};

	class GameEnd
	{
	public:
		GameEnd(const EndData& data);
		std::shared_ptr<Packet> toPacket();

	private:

		EndData data;
	};
}