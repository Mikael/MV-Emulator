#ifndef SHUTDOWN_SIMPLECOMMAND_HEADER
#define SHUTDOWN_SIMPLECOMMAND_HEADER

#include <string>
#include <charconv>
#include "../../Network/MainSession.h"
#include "../../Network/MainSessionManager.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ISimpleCommand.h"

namespace Main
{
	namespace Command
	{
		struct Shutdown : public Common::Command::ISimpleCommand<Main::Network::Session, Main::Network::SessionsManager>
		{
			Shutdown(const Common::Enums::PlayerGrade requiredGrade);

			void execute(Main::Network::Session& session, Main::Network::SessionsManager& sessionManager, Common::Network::Packet& response) override;

			void getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response) override;
		};
	}
}
#endif