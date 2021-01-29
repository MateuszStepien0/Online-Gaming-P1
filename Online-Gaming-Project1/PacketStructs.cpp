#include "PacketStructs.h"

namespace PS
{
	ChatMessage::ChatMessage(const std::string & msg)
		:m_message(msg)
	{
	}

	std::shared_ptr<Packet> ChatMessage::toPacket()
	{
		std::shared_ptr<Packet> p = std::make_shared<Packet>();
		p->Append(PacketType::MESSAGE);
		p->Append(m_message.size());
		p->Append(m_message);
		return p;
	}

	StateChange::StateChange(const GameState& state)
		:state(state)
	{
	}

	std::shared_ptr<Packet> StateChange::toPacket()
	{
		std::shared_ptr<Packet> p = std::make_shared<Packet>();
		p->Append(PacketType::STATECHANGE);
		p->Append(sizeof(state));
		p->Append(state);
		return p;
	}

	GameStart::GameStart(const StartData& data)
		:data(data)
	{
	}

	std::shared_ptr<Packet> GameStart::toPacket()
	{
		std::shared_ptr<Packet> p = std::make_shared<Packet>();
		p->Append(PacketType::PLAYERREADY);
		p->Append(sizeof(data));
		p->Append(data);
		return p;
	}

	PlayerUpdate::PlayerUpdate(const PlayerData& data)
		:data(data)
	{
	}

	std::shared_ptr<Packet> PlayerUpdate::toPacket()
	{
		std::shared_ptr<Packet> p = std::make_shared<Packet>();
		p->Append(PacketType::PLAYERUPDATE);
		p->Append(sizeof(data));
		p->Append(data);
		return p;
	}

	GameUpdate::GameUpdate(const std::array<sf::Vector2f, 3>& positions)
		:positions(positions)
	{
	}

	std::shared_ptr<Packet> GameUpdate::toPacket()
	{
		std::shared_ptr<Packet> p = std::make_shared<Packet>();
		p->Append(PacketType::GAMEUPDATE);
		p->Append(sizeof(positions));
		p->Append(positions);
		return p;
	}

	GameEnd::GameEnd(const EndData& data)
		:data(data)
	{
	}

	std::shared_ptr<Packet> GameEnd::toPacket()
	{
		std::shared_ptr<Packet> p = std::make_shared<Packet>();
		p->Append(PacketType::GAMEEND);
		p->Append(sizeof(data));
		p->Append(data);
		return p;
	}
}