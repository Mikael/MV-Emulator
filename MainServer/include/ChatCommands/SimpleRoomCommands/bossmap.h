#pragma once
#ifndef BOSS_MAP_SIMPLE_CHAT_COMMAND_HEADER
#define BOSS_MAP_SIMPLE_CHAT_COMMAND_HEADER

#include <string>
#include <charconv>
#include "../../Network/MainSession.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ISimpleRoomCommand.h"
#include "../../Classes/RoomsManager.h"

namespace Main
{
	namespace Command
	{
		struct BossMap : public Common::Command::ISimpleRoomCommand<Main::Network::Session, Main::Classes::RoomsManager>
		{
			BossMap(const Common::Enums::PlayerGrade requiredGrade);

			void execute(Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager, std::size_t roomNumber, Common::Network::Packet& response) override;

			void getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response) override;
		};
	}
}

#endif // BOSS_MAP_SIMPLE_CHAT_COMMAND_HEADER
