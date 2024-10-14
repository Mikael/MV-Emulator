#ifndef PLAYER_POSITION_HANDLER_H
#define PLAYER_POSITION_HANDLER_H

#include "../Network/CastSession.h"
#include "../Network/SessionsManager.h"
#include "../Structures/PlayerPositionFromServer.h"
#include "../Structures/SuicideStruct.h"
#include "../Utils/HostSuicideUtils.h"
// Removed logger include for direct printing

namespace Cast
{
    namespace Handlers
    {
        // Constants for packet sizes
        constexpr size_t PACKET_SIZE_BASIC = 28;
        constexpr size_t PACKET_SIZE_WITH_JUMP = 32;
        constexpr size_t PACKET_SIZE_WITH_BULLETS = 36;
        constexpr size_t PACKET_SIZE_COMPLETE = 40;

        inline void handlePlayerPosition(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            using namespace Cast::Structures;

            const auto fullSize = request.getFullSize();

            // Validate incoming packet size
            if (fullSize != PACKET_SIZE_BASIC && fullSize != PACKET_SIZE_WITH_JUMP &&
                fullSize != PACKET_SIZE_WITH_BULLETS && fullSize != PACKET_SIZE_COMPLETE) {
                std::cerr << "Invalid packet size: " << fullSize << std::endl; // Regular printing
                return;
            }

            // Extract basic player position information
            ClientPlayerInfoBasic playerPositionFromClient{};
            std::memcpy(&playerPositionFromClient, request.getData(), sizeof(playerPositionFromClient));

            Common::Network::Packet response;
            response.setTcpHeader(session.getId(), Common::Enums::NO_ENCRYPTION);
            response.setOrder(322);
            response.setOption(1);

            if (fullSize == PACKET_SIZE_WITH_BULLETS || fullSize == PACKET_SIZE_COMPLETE)
            {
                ClientPlayerInfoBullet playerPositionBullet{};
                std::memcpy(&playerPositionBullet, request.getData(), sizeof(playerPositionBullet));

                PlayerInfoResponseWithBullets playerInfoResponseWithBullets{};
                playerInfoResponseWithBullets.specificInfo.enableBullet = true;

                playerInfoResponseWithBullets.tick = playerPositionFromClient.matchTick;
                playerInfoResponseWithBullets.position = playerPositionFromClient.position;
                playerInfoResponseWithBullets.direction = playerPositionFromClient.direction;
                playerInfoResponseWithBullets.specificInfo.animation1 = playerPositionFromClient.animation1;
                playerInfoResponseWithBullets.specificInfo.animation2 = playerPositionFromClient.animation2;
                playerInfoResponseWithBullets.rotation1 = request.getExtra();
                playerInfoResponseWithBullets.rotation2 = request.getOption();
                playerInfoResponseWithBullets.rotation3 = playerPositionFromClient.rotation;
                playerInfoResponseWithBullets.specificInfo.sessionId = static_cast<std::uint32_t>(session.getId());
                playerInfoResponseWithBullets.bullets = playerPositionBullet.bulletStruct;
                playerInfoResponseWithBullets.currentWeapon = playerPositionBullet.bulletStruct.bullet4;

                response.setData(reinterpret_cast<std::uint8_t*>(&playerInfoResponseWithBullets), sizeof(playerInfoResponseWithBullets));
            }
            else if (fullSize == PACKET_SIZE_WITH_JUMP)
            {
                PlayerInfoBasicResponse playerInfoBasicResponse{};
                playerInfoBasicResponse.tick = playerPositionFromClient.matchTick;
                playerInfoBasicResponse.position = playerPositionFromClient.position;
                playerInfoBasicResponse.direction = playerPositionFromClient.direction;
                playerInfoBasicResponse.currentWeapon = playerPositionFromClient.weapon;
                playerInfoBasicResponse.specificInfo.animation1 = playerPositionFromClient.animation1;
                playerInfoBasicResponse.specificInfo.animation2 = playerPositionFromClient.animation2;
                playerInfoBasicResponse.rotation1 = request.getExtra();
                playerInfoBasicResponse.rotation2 = request.getOption();
                playerInfoBasicResponse.rotation3 = playerPositionFromClient.rotation;
                playerInfoBasicResponse.specificInfo.sessionId = static_cast<std::uint32_t>(session.getId());

                // Enable jump information
                PlayerInfoResponseWithJump playerInfoResponseWithJump{ playerInfoBasicResponse };
                ClientPlayerInfoJump playerPositionJump{};
                std::memcpy(&playerPositionJump, request.getData(), request.getDataSize());
                playerInfoResponseWithJump.jump = playerPositionJump.jumpStruct;

                response.setData(reinterpret_cast<std::uint8_t*>(&playerInfoResponseWithJump), sizeof(playerInfoResponseWithJump));
            }
            else if (fullSize == PACKET_SIZE_BASIC)
            {
                PlayerInfoBasicResponse playerInfoBasicResponse{};
                playerInfoBasicResponse.tick = playerPositionFromClient.matchTick;
                playerInfoBasicResponse.position = playerPositionFromClient.position;
                playerInfoBasicResponse.direction = playerPositionFromClient.direction;
                playerInfoBasicResponse.currentWeapon = playerPositionFromClient.weapon;
                playerInfoBasicResponse.specificInfo.animation1 = playerPositionFromClient.animation1;
                playerInfoBasicResponse.specificInfo.animation2 = playerPositionFromClient.animation2;
                playerInfoBasicResponse.rotation1 = request.getExtra();
                playerInfoBasicResponse.rotation2 = request.getOption();
                playerInfoBasicResponse.rotation3 = playerPositionFromClient.rotation;
                playerInfoBasicResponse.specificInfo.sessionId = static_cast<std::uint32_t>(session.getId());

                response.setData(reinterpret_cast<std::uint8_t*>(&playerInfoBasicResponse), sizeof(playerInfoBasicResponse));
            }

            // Broadcast response to other players in the room
            roomsManager.broadcastToRoomExceptSelf(session.getId(), response);
        }
    }
}

#endif // PLAYER_POSITION_HANDLER_H
