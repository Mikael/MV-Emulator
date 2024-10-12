#ifndef ROOM_START_HANDLER_HEADER
#define ROOM_START_HANDLER_HEADER

#include "../../Network/MainSession.h"
#include "../../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "Network/Packet.h"
#include "../../Classes/RoomsManager.h"

#include <boost/interprocess/shared_memory_object.hpp> 
#include <boost/interprocess/mapped_region.hpp> 
#include <chrono> 
#include <random>
#include <Utils/IPC_Structs.h>
#include "Utils/Logger.h"

#include "../../../Common/include/Enums/RoomEnums.h"

namespace Main
{
    namespace Handlers
    {
        enum RoomStartExtra
        {
            START_SUCCESS = 38,
        };

        std::uint64_t getUtcTimeMs()
        {
            const auto durationSinceEpoch = std::chrono::system_clock::now().time_since_epoch();
            return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(durationSinceEpoch).count());
        }

        std::uint32_t getRandomMapForGameMode(Common::Enums::GameModes mode)
        {
            std::vector<std::uint32_t> availableMaps = Common::Enums::getMapsForGameMode(mode);

            if (availableMaps.empty())
            {
                return static_cast<std::uint32_t>(Common::Enums::GameMaps::Random);
            }

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, availableMaps.size() - 1);

            return availableMaps[dis(gen)];
        }


        inline void handleRoomStart(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager,
            std::uint64_t timeSinceLastServerRestart)
        {
            Utils::Logger& logger = Utils::Logger::getInstance();

            // Find the room the session is in
            const auto foundRoom = roomsManager.getRoomByNumber(session.getRoomNumber());
            if (foundRoom == std::nullopt) return;
            auto& room = foundRoom.value().get();

            Common::Network::Packet response;
            response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
            auto selfUniqueId = session.getAccountInfo().uniqueId;

            // Player clicked "Start" to begin the match
            if (request.getExtra() == 38)
            {
                session.setIsInMatch(true);

                // REMOVE WHEN IMPLEMENTED!!
                auto gameMode = room.getRoomSettings().mode;
                if (gameMode == Common::Enums::AiBattle || gameMode == Common::Enums::BossBattle) return;

                auto roomSettings = room.getRoomSettings();

                if (roomSettings.map == Common::Enums::GameMaps::Random)
                {
                    auto randomMap = getRandomMapForGameMode(static_cast<Common::Enums::GameModes>(roomSettings.mode));

                    room.updateMap(randomMap);
                }

                room.startMatch(selfUniqueId);

                response.setOrder(request.getOrder());
                response.setExtra(38);
                response.setOption(room.getRoomSettings().map);
                response.setData(reinterpret_cast<std::uint8_t*>(&selfUniqueId), sizeof(selfUniqueId));
                room.broadcastToRoom(response);

                // Log the match start
                logger.log("The player " + session.getPlayerInfoAsString() + " is entering the match. "
                    + room.getRoomInfoAsString(), Utils::LogType::Normal, "Room::handleRoomStart");

                // Temporary fix for potential bugs with starting matches after relogging
                response.setOrder(125);
                response.setMission(0);
                response.setExtra(0);
                response.setOption(roomSettings.mode);
                auto settings = room.getRoomSettingsUpdate();
                response.setData(reinterpret_cast<std::uint8_t*>(&settings), sizeof(settings));
                session.asyncWrite(response);

                // If the player is the host, notify other services about the room map and mode
                if (room.isHost(selfUniqueId))
                {
                    Utils::MapInfo mapInfo{ room.getRoomSettings().map }; // Now using the updated map
                    Utils::IPCManager::ipc_mainToCast(mapInfo, std::to_string(room.getRoomNumber()), "map_info");

                    logger.log("The player " + session.getPlayerInfoAsString() + " is also host. Main=>Cast notification about room map and mode info sent. "
                        + room.getRoomInfoAsString(), Utils::LogType::Normal, "Room::handleRoomStart");
                }
            }
			else if (request.getExtra() == 41)
			{
				if (room.isHost(selfUniqueId))
				{

					response.setOrder(258);
					response.setExtra(1 /* 5 == infinite loading for host ??*/); 
					response.setOption(0);
					struct Response
					{
						std::uint64_t tick = 0; 
					};
					Response respMessage;
                    respMessage.tick = getUtcTimeMs() - timeSinceLastServerRestart;
					response.setData(reinterpret_cast<std::uint8_t*>(&respMessage), sizeof(respMessage));
					room.broadcastToRoom(response);

					logger.log("The player " + session.getPlayerInfoAsString() + " is also host. RoomTick set and broadcast to room. "
						+ room.getRoomInfoAsString(), Utils::LogType::Normal, "Room::handleRoomStart");
				}
                else
                {
                    response.setOrder(415);
                    response.setExtra(1);
                    response.setData(reinterpret_cast<std::uint8_t*>(&selfUniqueId), sizeof(selfUniqueId));
                    response.setMission(0);
                    response.setOption(0);
                    room.broadcastToRoom(response);

                    logger.log("The player " + session.getPlayerInfoAsString() + " is NOT host. Notifying other players about their match join. "
                        + room.getRoomInfoAsString(), Utils::LogType::Normal, "Room::handleRoomStart");
                }
            }
        }
    }
}

#endif
