#ifndef PLAYER_STATE_HANDLER_H
#define PLAYER_STATE_HANDLER_H

#include "../../Network/MainSession.h"
#include "../../Network/MainSessionManager.h"
#include "Network/Packet.h"
#include <ConstantDatabase/Structures/SetItemInfo.h>
#include <Utils/Utils.h>

namespace Main
{
	namespace Handlers
	{
		constexpr int TOTAL_CAPSULE_ITEMS = 5;
		constexpr int PLAYER_STATE_CHANGE_NOTIFICATION_ORDER = 312;
		constexpr int ITEM_EQUIP_NOTIFICATION_ORDER = 414;
		constexpr int STATE_READY = 17;

		inline void handlePlayerState(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
		{
			// If the player is trading or already in the requested state, return
			auto newState = static_cast<Common::Enums::PlayerState>(request.getOption());
			if (session.getPlayerState() == Common::Enums::PlayerState::STATE_TRADE || session.getPlayerState() == newState)
				return;

			// Update the player's state
			session.setPlayerState(newState);

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);

			// Handle entering capsule state (e.g., showing capsule items)
			if (newState == Common::Enums::PlayerState::STATE_CAPSULE)
			{
				session.sendCurrency();  // Send the player's currency
				response.setOrder(83);   // Capsule order, replace magic number with a named constant if available

				// Retrieve capsule items (ensure we handle cases where less than TOTAL_CAPSULE_ITEMS are returned)
				Main::ConstantDatabase::CdbUtil cdbUtil;
				auto capsuleItems = cdbUtil.getCapsuleItems(TOTAL_CAPSULE_ITEMS);
				if (capsuleItems.size() < TOTAL_CAPSULE_ITEMS)
				{
					// Error handling: Log the issue or provide a fallback
					std::cerr << "Error: Less than " << TOTAL_CAPSULE_ITEMS << " capsule items retrieved." << std::endl;
					return;
				}
				response.setData(reinterpret_cast<std::uint8_t*>(capsuleItems.data()), capsuleItems.size() * sizeof(Main::Structures::CapsuleList));
				response.setOption(static_cast<std::uint32_t>(capsuleItems.size()));
				session.asyncWrite(response);
			}

			// Find the room the player is in
			auto foundRoom = roomsManager.getRoomByNumber(session.getRoomNumber());
			if (foundRoom == std::nullopt) return;
			auto& room = foundRoom->get();  // Use get() method instead of the operator for clarity
			const auto& accountInfo = session.getAccountInfo();

			// Notify other players in the room of the player's state change
			response.setOrder(PLAYER_STATE_CHANGE_NOTIFICATION_ORDER);
			response.setOption(session.getPlayerState());
			auto uniqueId = accountInfo.uniqueId;
			response.setData(reinterpret_cast<std::uint8_t*>(&uniqueId), sizeof(uniqueId));
			room.broadcastToRoom(response);
			room.setStateFor(uniqueId, session.getPlayerState());

			// Handle state transitions like ready/waiting and broadcast new equipped items/character
			if (newState == Common::Enums::PlayerState::STATE_READY || newState == Common::Enums::PlayerState::STATE_WAITING)
			{
				if (session.getRoomNumber())
				{
					room.updatePlayerInfo(&session);

					// Structure to send equipped item info to other players
					using setItems = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::SetItemInfo>;
					struct Resp
					{
						Main::Structures::UniqueId uniqueId;
						Main::Structures::BasicEquippedItem items;
					} resp;
					resp.uniqueId = accountInfo.uniqueId;

					// Process equipped items
					for (const auto& [type, item] : session.getEquippedItems())
					{
						// Process SET items
						if (type >= Common::Enums::ItemType::SET)
						{
							auto entry = setItems::getInstance().getEntry("si_id", (item.id >> 1));
							if (entry != std::nullopt)
							{
								for (auto currentTypeNotNull : Common::Utils::getPartTypesWhereSetItemInfoTypeNotNull(*entry))
								{
									resp.items.equippedItems[currentTypeNotNull].equippedItemId = item.id;
									resp.items.equippedItems[currentTypeNotNull].type = currentTypeNotNull;
								}
							}
						}
						// Process HAIR, ACCESSORY, etc.
						else if (type >= Common::Enums::ItemType::HAIR && type <= Common::Enums::ItemType::ACC_BACK)
						{
							resp.items.equippedItems[type] = item;
						}
						// Process WEAPONS
						else if (type >= Common::Enums::ItemType::MELEE && type <= Common::Enums::ItemType::GRENADE)
						{
							resp.items.equippedWeapons[type - 10] = item;  // Offset for weapon types
						}
					}

					// Send item/character info to other players in the room
					response.setOrder(ITEM_EQUIP_NOTIFICATION_ORDER);
					response.setOption(STATE_READY); // Example option for ready state
					response.setExtra(accountInfo.latestSelectedCharacter);
					response.setData(reinterpret_cast<std::uint8_t*>(&resp), sizeof(resp));
					roomsManager.broadcastToRoomExceptSelf(session.getRoomNumber(), accountInfo.uniqueId, response);
				}
			}
		}
	}
}

#endif
