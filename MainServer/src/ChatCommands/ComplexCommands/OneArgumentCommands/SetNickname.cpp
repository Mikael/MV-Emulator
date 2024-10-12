#include <string>
#include <charconv>
#include <regex>
#include <set>
#include <iostream> // Debugging
#include "../../../../include/Network/MainSession.h"
#include "../../../../include/Network/MainSessionManager.h"
#include "../../../../include/ChatCommands/ComplexCommands/OneArgumentCommands/SetNickname.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ComplexCommands/OneArgumentCommands/AbstractOneArgNonNumericalCommand.h"

namespace Main
{
    namespace Command
    {
        SetNickname::SetNickname(const Common::Enums::PlayerGrade requiredGrade)
            : AbstractOneArgNonNumericalCommand<Main::Network::Session, Main::Network::SessionsManager>{ requiredGrade }
        {
        }

        void SetNickname::execute(const std::string& providedCommand, Main::Network::Session& session, Main::Network::SessionsManager& sessionManager, Common::Network::Packet& response)
        {
            if (!parseCommand(providedCommand))
            {
                this->m_confirmationMessage += "error: parsing error";
                sendConfirmation(response, session);
                return;
            }

            std::cout << "Parsed nickname: " << m_value << std::endl; // Debugging

            if (!isValidNickname(m_value))
            {
                this->m_confirmationMessage += "error: nickname must contain only letters and numbers";
                sendConfirmation(response, session);
                return;
            }

            if (m_value.size() >= 15)
            {
                this->m_confirmationMessage += "error: nickname must contain less than 15 characters";
                sendConfirmation(response, session);
                return;
            }

            if (isRestrictedName(m_value))
            {
                this->m_confirmationMessage += "error: nickname is not allowed";
                sendConfirmation(response, session);
                return;
            }

            if (isNicknameTaken(m_value, sessionManager)) //not implemented
            {
                this->m_confirmationMessage += "error: nickname is already taken";
                sendConfirmation(response, session);
                return;
            }

            session.setPlayerName(m_value.c_str());

            // Debugging
            std::cout << "Nickname set to: " << m_value << std::endl;

            this->m_confirmationMessage += "Your name has been updated. Please relog.";
            sendConfirmation(response, session);
        }

        void SetNickname::getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response)
        {
            this->m_confirmationMessage += "/setname <nickname>";
            sendConfirmation(response, session);
        }

        bool SetNickname::isValidNickname(const std::string& nickname)
        {
            const std::regex validNicknameRegex("[A-Za-z0-9]+");
            return std::regex_match(nickname, validNicknameRegex);
        }

        bool SetNickname::isRestrictedName(const std::string& nickname)
        {
            static const std::set<std::string> restrictedNames = { "gm", "mod", "admin", "adm", "md", "microvolts" };

            std::string lowerNickname = nickname;
            std::transform(lowerNickname.begin(), lowerNickname.end(), lowerNickname.begin(),
                [](unsigned char c) { return std::tolower(c); });

            return restrictedNames.find(lowerNickname) != restrictedNames.end();
        }

        bool SetNickname::isNicknameTaken(const std::string& nickname, Main::Network::SessionsManager& sessionManager)
        {
            // todo
            return false;
        }
    }
}