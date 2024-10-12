#ifndef MUTEROOM_SIMPLE_CHAT_COMMAND_HEADER
#define MUTEROOM_SIMPLE_CHAT_COMMAND_HEADER

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
		struct Muteroom : public Common::Command::ISimpleRoomCommand<Main::Network::Session, Main::Classes::RoomsManager>
		{
			Muteroom(const Common::Enums::PlayerGrade requiredGrade);

			void execute(Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager, std::size_t roomNumber, Common::Network::Packet& response) override;

			void getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response) override;
		};
	}
}
#endif