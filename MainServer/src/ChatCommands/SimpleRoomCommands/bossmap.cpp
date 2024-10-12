#include <string>
#include <charconv>
#include "../../../include/Network/MainSession.h"
#include "../../../include/Classes/RoomsManager.h"
#include "../../../include/ChatCommands/SimpleRoomCommands/BossMap.h"  // Adjust the include path as necessary
#include "../../../include/Classes/Room.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ISimpleRoomCommand.h"

#include <boost/interprocess/shared_memory_object.hpp> 
#include <boost/interprocess/mapped_region.hpp> 
#include <chrono> 

namespace Main
{
	namespace Command
	{
		BossMap::BossMap(const Common::Enums::PlayerGrade requiredGrade)
			: ISimpleRoomCommand{ requiredGrade }
		{
		}

		void BossMap::execute(Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager, std::size_t roomNumber, Common::Network::Packet& response)
		{
			this->m_confirmationMessage += "success";

			auto foundRoom = roomsManager.getRoomByNumber(roomNumber);
			if (foundRoom == std::nullopt)
			{
				this->m_confirmationMessage += "error: room not found.";
			}
			else
			{
				auto& room = foundRoom.value().get();
				room.updateMap(45);  // Update the room's map to BossBattleMap (ID: 45)
				this->m_confirmationMessage += " and the map has been updated to BossBattleMap.";
			}
			sendConfirmation(response, session);
		}

		void BossMap::getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response)
		{
			this->m_confirmationMessage += "/bossmap: changes the map to BossBattleMap.";
			sendConfirmation(response, session);
		}
	};
}
