#ifndef DETAILS_HEADER_H
#define DETAILS_HEADER_H

#include <ConstantDatabase/Structures/SetItemInfo.h>
#include <Utils/Utils.h>
#include "Network/MainSession.h"
#include "Classes/RoomsManager.h"
#include "Network/Packet.h"

#include <chrono>

namespace Main
{
	namespace Details
	{
		namespace Constants
		{
			constexpr inline std::uint32_t maxEquippedItems = 17;
		}

		enum Orders
		{
			PLAYER_STATE_NOTIFICATION = 312,
			PLAYER_ITEMS_BROADCAST = 414,
		};

		template<typename T>
		T parseData(const Common::Network::Packet& request, std::uint32_t offset = 0)
		{
			// this is not always necessarily true:
			// assert(sizeof(T) == request.getDataSize());

			T t;
			std::memcpy(&t, request.getData() + offset, request.getDataSize());
			return t;
		}

		inline void sendMessage(const std::string& message, Main::Network::Session& session)
		{
			Common::Network::Packet response;
			response.setTcpHeader(session.getId(), Common::Enums::USER_LARGE_ENCRYPTION);
			std::string m_confirmationMessage{ std::string(16, '0') };
			m_confirmationMessage += message;
			response.setOrder(316);
			response.setExtra(1);
			response.setData(reinterpret_cast<std::uint8_t*>(m_confirmationMessage.data()), m_confirmationMessage.size());
			response.setOption(m_confirmationMessage.size());
			session.asyncWrite(response);
		}

		inline void sendPlayerState(Main::Network::Session& session, Main::Structures::UniqueId uniqueId, Main::Classes::Room& room, std::uint32_t state = 11)
		{
			Common::Network::Packet response;
			response.setTcpHeader(session.getId(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setCommand(312, 0, 0, state);
			response.setData(reinterpret_cast<std::uint8_t*>(&uniqueId), sizeof(uniqueId));
			room.broadcastToRoom(response);
		}

		inline std::uint64_t getUtcTimeMs()
		{
			const auto durationSinceEpoch = std::chrono::system_clock::now().time_since_epoch();
			return static_cast<std::uint64_t>(duration_cast<std::chrono::milliseconds>(durationSinceEpoch).count());
		}
	}
}

#endif