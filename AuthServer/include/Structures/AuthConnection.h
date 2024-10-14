#ifndef AUTH_INITIAL_PACKET_STRUCTURE_H
#define AUTH_INITIAL_PACKET_STRUCTURE_H

#ifdef WIN32
#include <corecrt.h>
#else
#define __time32_t uint32_t
#endif

#include <ctime>
#include <httplib.h> // Use httplib for HTTP requests
#include <nlohmann/json.hpp> // Include JSON library
#include <iostream>

#include "Protocol/TcpHeader.h"
#include "Protocol/CommandHeader.h"
#include "Enums/MiscellaneousEnums.h"
#include "Enums/ExtrasEnums.h"

namespace Auth
{
    namespace Structures
    {
#pragma pack(push, 1)
        struct ConnectionPacket
        {
            Common::Protocol::TcpHeader tcpHeader{};
            Common::Protocol::CommandHeader commandHeader{};
            std::int32_t key{};
            __time32_t timestamp{};

            ConnectionPacket()
                : key{ static_cast<std::int32_t>(rand() + 1) }
                , timestamp{ static_cast<__time32_t>(std::time(0)) }
            {
                tcpHeader.initialize(0, Common::Enums::EncryptionType::NO_ENCRYPTION, sizeof(ConnectionPacket));

                // Commenting out the maintenance check for now
                /*
                bool isMaintenance = checkMaintenanceStatus();

                // Set the command header based on the maintenance status
                if (isMaintenance)
                {
                    commandHeader.initialize(401, Common::Random::generateRandomOption(), static_cast<int>(Common::Enums::AUTH_MAINTENANCE), Common::Random::generateRandomMission());
                }
                else
                {
                    commandHeader.initialize(401, Common::Random::generateRandomOption(), static_cast<int>(Common::Enums::AUTH_SUCCESS), Common::Random::generateRandomMission());
                }
                */

                // Always set to AUTH_SUCCESS for now
                commandHeader.initialize(401, Common::Random::generateRandomOption(), static_cast<int>(Common::Enums::AUTH_SUCCESS), Common::Random::generateRandomMission());
            }
        };
#pragma pack(pop)
    }
}

#endif
