#ifndef ROOM_LEAVE_HANDLER_H
#define ROOM_LEAVE_HANDLER_H

#include "../../Network/MainSession.h"
#include "../../Classes/RoomsManager.h"
#include "Network/Packet.h"
#include "../../Network/MainSessionManager.h"
#include "Utils/Logger.h"
#include "Utils/IPC_Structs.h"

namespace Main
{
	namespace Handlers
	{
		enum RoomLeaveExtra
		{
			LEAVE_ERROR = 0, // doesn't leave the room
			LEAVE_NORMAL = 1,
			LEAVE_UNKNOWN = 0x2F, // seems normal exit?
			LEAVE_UNKNOWN1 = 0x15, // seems normal exit?
			LEAVE_UNKNOWN2 = 4, // seems normal exit?
			LEAVE_BREAKROOM = 0x1B,
			LEAVE_KICKED_BY_HOST = 0x2A,
			LEAVE_KICKED_BY_MOD = 0x23,
			LEAVE_VOTEKICK_COARSE_LANGUAGE = 0x27,
		};

		enum ClientExtra
		{
			KICK_PLAYER = 28,
		};

		// Notifies other players that the player with uniqueId left the room
		inline void notifyRoomPlayerLeaves(Main::Structures::UniqueId uniqueId, Main::Classes::Room& room)
		{
			Common::Network::Packet response;
			response.setTcpHeader(0, Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(422);
			response.setOption(1); // Unsure whether this is the new hostIDX, or the team of the player that left!
			response.setData(reinterpret_cast<std::uint8_t*>(&uniqueId), sizeof(uniqueId));
			room.broadcastToRoom(response);
		}

		// Handle player leaving the room
		inline void handleRoomLeave(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager, Main::Classes::RoomsManager& roomsManager)
		{
			Utils::Logger& logger = Utils::Logger::getInstance();

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());

			auto roomOpt = roomsManager.getRoomByNumber(session.getRoomNumber());
			if (roomOpt == std::nullopt)
			{
				response.setExtra(RoomLeaveExtra::LEAVE_ERROR);
				session.asyncWrite(response);
				return;
			}
			auto& room = roomOpt->get();

			if (request.getExtra() == ClientExtra::KICK_PLAYER)
			{
				response.setExtra(RoomLeaveExtra::LEAVE_KICKED_BY_HOST);
				Main::Structures::UniqueId uniqueId;
				std::memcpy(&uniqueId, request.getData(), sizeof(uniqueId));
				uniqueId.server = 4;
				auto* targetSession = sessionsManager.getSessionBySessionId(uniqueId.session);
				if (targetSession)
				{
					if (targetSession->getAccountInfo().playerGrade >= session.getAccountInfo().playerGrade)
						return; // Prevent kicking same or higher grade accounts

					room.removePlayer(targetSession, RoomLeaveExtra::LEAVE_KICKED_BY_HOST);
					room.addKickedPlayer(targetSession->getAccountInfo().accountID);
				}
			}
			else
			{
				response.setExtra(RoomLeaveExtra::LEAVE_NORMAL);

				if (room.isSessionInList(&session))
				{
					const bool mustRoomBeClosed = room.removePlayer(&session, RoomLeaveExtra::LEAVE_NORMAL);

					if (mustRoomBeClosed)
					{
						roomsManager.removeRoom(room.getRoomNumber());
					}
				}
			}
		}
	}
}

#endif
