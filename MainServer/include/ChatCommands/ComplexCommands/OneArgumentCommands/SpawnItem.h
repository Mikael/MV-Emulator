#ifndef MAIN_COMMAND_SPAWNITEM_H
#define MAIN_COMMAND_SPAWNITEM_H

#include <string>
#include <charconv>
#include "../../../Network/MainSession.h"
#include "../../../Network/MainSessionManager.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ComplexCommands/OneArgumentCommands/AbstractOneArgNumericalCommand.h"
#include "../../../../include/Structures/Item/SpawnedItem.h"

namespace Main
{
    namespace Command
    {
        struct SpawnItem : public Common::Command::AbstractOneArgNumericalCommand<Main::Network::Session, Main::Network::SessionsManager>
        {
            SpawnItem(const Common::Enums::PlayerGrade requiredGrade);
            void execute(const std::string& providedCommand, Main::Network::Session& session, Main::Network::SessionsManager& sessionManager, Common::Network::Packet& response) override;
            void getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response) override;
        };
    }
}

#endif
