#ifndef PING_HANDLER_H
#define PING_HANDLER_H

#include <unordered_map>
#include "../../Network/MainSession.h"
#include "../../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "Network/Packet.h"
#include "../../../include/Classes/RoomsManager.h"
#include "../../Structures/ClientData/Structures.h"

namespace Main
{
    namespace Handlers
    {
        inline void handlePing(const Common::Network::Packet& request,
            Main::Network::Session& session,
            Main::Classes::RoomsManager& roomsManager,
            const Main::ClientData::Ping& pingData)
        {
            if (request.getMission() == 1)
            {
                auto optionalRoom = roomsManager.getRoomByNumber(session.getRoomNumber());

                if (optionalRoom.has_value())
                {
                    Main::Classes::Room& room = optionalRoom.value().get();

                    // Update the ping value in the session
                    session.setPing(pingData.ping);

                    // Prepare the response structure
                    struct Resp
                    {
                        Main::ClientData::Ping clientData;
                        Main::Structures::UniqueId uniqueId;
                    };

                    Resp resp{ pingData, session.getAccountInfo().uniqueId };

                    // Create a packet to send the response
                    Common::Network::Packet response;
                    response.setTcpHeader(request.getSession(), Common::Enums::NO_ENCRYPTION);
                    response.setCommand(request.getOrder() + 1, request.getMission() + 1, 0, request.getOption());
                    response.setData(reinterpret_cast<std::uint8_t*>(&resp), sizeof(resp));

                    // Broadcast to the room, excluding self
                    room.broadcastToRoomExceptSelf2(std::move(response), resp.uniqueId);
                }
            }
        }
    }
}

#endif
