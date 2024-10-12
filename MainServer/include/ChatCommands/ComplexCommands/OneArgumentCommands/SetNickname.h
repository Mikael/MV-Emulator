#ifndef SETNICKNAME_COMPLEXCOMMAND_HEADER
#define SETNICKNAME_COMPLEXCOMMAND_HEADER

#include <string>
#include <regex>
#include <charconv>
#include "../../../Network/MainSession.h"
#include "../../../Network/MainSessionManager.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ComplexCommands/OneArgumentCommands/AbstractOneArgNonNumericalCommand.h"

namespace Main
{
    namespace Command
    {
        struct SetNickname : public Common::Command::AbstractOneArgNonNumericalCommand<Main::Network::Session, Main::Network::SessionsManager>
        {
            SetNickname(const Common::Enums::PlayerGrade requiredGrade);

            void execute(const std::string& providedCommand, Main::Network::Session& session, Main::Network::SessionsManager& sessionManager, Common::Network::Packet& response) override;

            void getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response) override;

        private:
            bool isValidNickname(const std::string& nickname);
            bool isNicknameTaken(const std::string& nickname, Main::Network::SessionsManager& sessionManager);
            bool isRestrictedName(const std::string& nickname);
        };
    }
}
#endif