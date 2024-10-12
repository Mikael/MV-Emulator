#ifndef PING_HANDLER_H
#define PING_HANDLER_H

#include <unordered_map> // Include the unordered_map header
#include "../../Network/MainSession.h"
#include "../../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "Network/Packet.h"
#include "../../../include/Classes/RoomsManager.h"
#include "../../Structures/ClientData/Structures.h"

namespace Main
{
    namespace Handlers 
    {
        inline void handlePing(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager,
            const Main::ClientData::Ping& pingData)
        {
            if (request.getMission() == 1)  // Ensure we're handling the correct mission type
            {
                // Get the room the user (session) is in, by room number
                auto optionalRoom = roomsManager.getRoomByNumber(session.getRoomNumber());

                // If the room exists, process the ping
                if (optionalRoom.has_value())
                {
                    Main::Classes::Room& room = optionalRoom.value().get();

                    // Update the ping value in the session
                    session.setPing(pingData.ping);

                    // Prepare the response structure
                    struct Resp
                    {
                        Main::ClientData::Ping clientData;       // Including Ping data in response (optional)
                        Main::Structures::UniqueId uniqueId{};   // Session/account's unique ID
                    };

                    // Create the response with Ping data and uniqueId
                    Resp resp{ pingData, session.getAccountInfo().uniqueId };

                    // Create a packet to send the response
                    Common::Network::Packet response;
                    response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);  // Set TCP header with session and encryption type
                    response.setCommand(request.getOrder() + 1, request.getMission() + 1, 0, request.getOption());  // Set command with updated order and mission
                    response.setData(reinterpret_cast<std::uint8_t*>(&resp), sizeof(resp));  // Attach the response data

                    // Broadcast the response to everyone in the room except the sender
                    room.broadcastToRoomExceptSelf(response, resp.uniqueId);
                }
            }
        }
    }
}

#endif
