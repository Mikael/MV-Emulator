#ifndef AUTH_AUTHORIZATION_HANDLER_H
#define AUTH_AUTHORIZATION_HANDLER_H

#include <string>
#include <functional>
#include "Network/Session.h"
#include "Network/Packet.h"
#include "../Database/DbPlayerInfo.h"
#include "../Player/AuthPlayerManager.h"
#include <limits>
#include "../include/Structures/AuthConnection.h"

namespace Auth
{
    namespace Handlers
    {
        inline void handleAuthUserInformation(const Common::Network::Packet& request, Common::Network::Session& session, Auth::Persistence::PersistentDatabase& database)
        {
            const std::string username = std::string(reinterpret_cast<const char*>(request.getData() + 48));
            const std::string password = std::string(reinterpret_cast<const char*>(request.getData() + 4));

            // Proceed with normal authentication process
            auto pair = database.getPlayerInfo(username, password);
            Common::Network::Packet response = pair.first;

            // Prepare response packet
            response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
            response.setOrder(22);
            response.setMission(0);

            // If login fails, send response and return
            if (response.getExtra() != Auth::Enums::Login::SUCCESS)
            {
                response.setData(nullptr, 0);
                session.asyncWrite(response);
                return;
            }

            const int nicknameOffset = 16;
            const int nicknameMaxLength = 32;
            std::string nickname(reinterpret_cast<const char*>(response.getData() + nicknameOffset), nicknameMaxLength);
            nickname.erase(nickname.find_last_not_of('\0') + 1);

            Auth::Structures::ConnectionPacket connectionPacket;

            std::cout << "User " << username << " logged in successfully." << std::endl;
            std::cout << "Nickname: " << nickname << std::endl;
            std::cout << "Connection key: " << connectionPacket.key << std::endl;
            std::cout << "Connection timestamp: " << connectionPacket.timestamp << std::endl;

            Common::Network::Packet connectionResponse;
            connectionResponse.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
            connectionResponse.setOrder(23);
            connectionResponse.setData(reinterpret_cast<uint8_t*>(&connectionPacket), sizeof(connectionPacket));

            session.asyncWrite(connectionResponse);
            session.asyncWrite(response);


            Auth::Player::AuthPlayerManager::getInstance().addPlayer(session.getId(), pair.second);
        }

    }
}

#endif // AUTH_AUTHORIZATION_HANDLER_H
