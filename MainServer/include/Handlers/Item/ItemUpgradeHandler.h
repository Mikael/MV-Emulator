#ifndef ITEM_ENERGY_INSERTION_HANDLER_H
#define ITEM_ENERGY_INSERTION_HANDLER_H

#include "../../../include/Network/MainSession.h"
#include "../../../include/Network/MainSessionManager.h"
#include "Network/Packet.h"
#include "../../Structures/PlayerLists/Friend.h"
#include "DeleteItemHandler.h"
#include "../../Structures/Item/SpawnedItem.h"
#include <random>
#include <iostream> // For error logging, can be replaced with a more robust logger

namespace Main
{
	namespace Handlers
	{
		// Enum for various outcomes of the item upgrade
		enum ItemUpgradeExtra
		{
			UPGRADE_SUCCESS = 1,
			UPGRADE_FAIL = 6,
			ITEM_MUST_BE_REPAIRED_FIRST = 8,
			NOT_ENOUGH_MP_FOR_UPGRADE = 14,
			ENERGY_ADD = 32,
		};

		// Constants
		constexpr int FAIL_RATE_PERCENT = 15;
		constexpr int UNUSED_DATA_SIZE = 72;   // Size of the unused vector during an upgrade
		constexpr int ENERGY_ACK_EXTRA = 0;    // Extra value to indicate energy insertion
		constexpr int UPGRADE_REQUEST_EXTRA = 37;
		constexpr std::size_t ENERGY_OFFSET = 8; // Offset to read the used energy value from the request

		inline void handleItemUpgrade(const Common::Network::Packet& request, Main::Network::Session& session)
		{
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());
			response.setOption(request.getOption());
			response.setMission(request.getMission());

			const auto receivedExtra = request.getExtra();
			const auto& accountInfo = session.getAccountInfo();

			// Client ACK: user adds energy to a weapon
			if (receivedExtra == ENERGY_ACK_EXTRA)
			{
				std::uint32_t usedEnergy;
				std::memcpy(&usedEnergy, request.getData() + ENERGY_OFFSET, sizeof(std::uint32_t));
				Main::Structures::ItemSerialInfo itemSerialInfo{};
				std::memcpy(&itemSerialInfo, request.getData(), sizeof(itemSerialInfo));

				// Security check: is the user trying to use more energy than they have?
				if (usedEnergy > accountInfo.battery)
				{
					std::cerr << "Energy insertion failed: Insufficient battery. Used: " << usedEnergy << ", Available: " << accountInfo.battery << std::endl;
					return;
				}

				// Add energy to the item and respond to the client
				if (!session.addEnergyToItem(itemSerialInfo, usedEnergy))
				{
					std::cerr << "Error: Failed to add energy to item." << std::endl;
					return;
				}

				response.setExtra(ItemUpgradeExtra::ENERGY_ADD);
				response.setData(reinterpret_cast<std::uint8_t*>(const_cast<std::uint8_t*>(request.getData())), sizeof(usedEnergy) + sizeof(Main::Structures::ItemSerialInfo));
				session.asyncWrite(response);
				return;
			}
			// Handle item upgrade request
			else if (receivedExtra == UPGRADE_REQUEST_EXTRA)
			{
				Main::Structures::ItemSerialInfo itemSerialInfo{};
				std::memcpy(&itemSerialInfo, request.getData(), sizeof(itemSerialInfo));
				std::vector<std::uint8_t> unused(UNUSED_DATA_SIZE);

				// Find the item by serial info
				auto foundItem = session.findItemBySerialInfo(itemSerialInfo);
				if (foundItem == std::nullopt)
				{
					std::cerr << "Error: Item not found for upgrade." << std::endl;
					return;
				}
				Main::ConstantDatabase::CdbUtil cdbCurrentId(foundItem->id);

				// Random fail rate: 15% chance of failure
				static std::mt19937 gen(std::random_device{}());  // Static to avoid reseeding
				std::uniform_int_distribution<int> dist(1, 100);
				int randomRoll = dist(gen);
				if (randomRoll <= FAIL_RATE_PERCENT)
				{
					// Upgrade failed
					response.setExtra(ItemUpgradeExtra::UPGRADE_FAIL);
					response.setData(unused.data(), unused.size());
					session.asyncWrite(response);

					// Delete old item and respawn a new one (with energy reset)
					session.deleteItem(itemSerialInfo);
					session.spawnItem(foundItem->id, itemSerialInfo);
					return;
				}

				// Upgrade succeeded
				response.setExtra(ItemUpgradeExtra::UPGRADE_SUCCESS);
				response.setData(unused.data(), unused.size());
				session.asyncWrite(response);

				// Handle MP (micro-points) requirement for the upgrade
				const auto mission = request.getMission();
				const auto hasParent = cdbCurrentId.hasParentId();
				if (hasParent == std::nullopt)
				{
					std::cerr << "Error: Failed to retrieve parent ID for item upgrade." << std::endl;
					return;
				}

				// Calculate the ID for the upgraded item
				const std::uint32_t toAdd = *hasParent ? 1 : (mission * 10 + 1);
				Main::ConstantDatabase::CdbUtil cdbUtil(foundItem->id + toAdd);
				auto mpNeededForUpgrade = cdbUtil.getMpNeededForUpgrade();
				if (mpNeededForUpgrade == std::nullopt)
				{
					std::cerr << "Error: Could not retrieve MP required for upgrade." << std::endl;
					return;
				}

				// Check if user has enough MP for the upgrade
				if (accountInfo.microPoints < *mpNeededForUpgrade)
				{
					response.setExtra(ItemUpgradeExtra::NOT_ENOUGH_MP_FOR_UPGRADE);
					session.asyncWrite(response);
					return;
				}

				// Delete the old item and respawn the upgraded one
				session.deleteItem(itemSerialInfo);
				session.spawnItem(foundItem->id + toAdd, itemSerialInfo);
				session.setAccountMicroPoints(accountInfo.microPoints - *mpNeededForUpgrade);
				session.sendCurrency();
			}
		}
	}
}

#endif
