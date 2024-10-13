#ifndef BOUGHT_ITEM_HANDLER_H
#define BOUGHT_ITEM_HANDLER_H

#include <unordered_map>
#include <vector>
#include "../../../include/Network/MainSession.h"
#include "../../Structures/Item/MainBoughtItem.h"
#include "../../CdbUtils.h"
#include "../../MainEnums.h"
#include "../../Structures/Item/MainItem.h"
#include "../../Structures/AccountInfo/MainAccountInfo.h"

namespace Main
{
	namespace Handlers
	{
		inline void removeAmountByCurrencyType(Main::Enums::ItemCurrencyType currencyType, std::uint32_t amountToRemove, Main::Network::Session& session,
			const Main::Structures::AccountInfo& accountInfo)
		{
			switch (currencyType)
			{
			case Main::Enums::ITEM_MP:
				session.setAccountMicroPoints(accountInfo.microPoints - amountToRemove);
				break;
			case Main::Enums::ITEM_RT:
				session.setAccountRockTotens(accountInfo.rockTotens - amountToRemove);
				break;
			}
		}

		inline bool hasSufficientCurrency(const Main::Structures::AccountInfo& accountInfo,
			const std::unordered_map<Main::Enums::ItemCurrencyType, std::uint32_t>& totalCurrencySpentByType)
		{
			for (const auto& [currencyType, totalSpent] : totalCurrencySpentByType)
			{
				if ((currencyType == Main::Enums::ITEM_MP && totalSpent > accountInfo.microPoints) ||
					(currencyType == Main::Enums::ITEM_RT && totalSpent > accountInfo.rockTotens))
				{
					return false;
				}
			}
			return true;
		}

		inline void handleBoughtItem(const Common::Network::Packet& request, Main::Network::Session& session)
		{
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());
			response.setOption(request.getOption()); // number of items bought

			const auto& accountInfo = session.getAccountInfo();
			std::unordered_map<Main::Enums::ItemCurrencyType, std::uint32_t> totalCurrencySpentByType;

			if (request.getMission() == 1) // Renewing expired item
			{
				std::vector<Main::Structures::BoughtItemToProlong> itemsToProlong;
				for (std::uint32_t idx = 0; idx < request.getOption(); ++idx)
				{
					Main::Structures::ItemSerialInfo itemSerialInfoToProlong;
					std::memcpy(&itemSerialInfoToProlong, request.getData() + idx * 8, sizeof(itemSerialInfoToProlong));
					auto item = session.findItemBySerialInfo(itemSerialInfoToProlong);
					if (!item) continue;

					const Main::ConstantDatabase::CdbUtil cdb(item->id);
					const auto amountByCurrencyType = cdb.getItemPrice();
					totalCurrencySpentByType[amountByCurrencyType->first] += amountByCurrencyType->second;

					Main::Structures::BoughtItemToProlong boughtItemToProlong{ itemSerialInfoToProlong };
					itemsToProlong.push_back(boughtItemToProlong);
					session.replaceItem(item->serialInfo.itemNumber, boughtItemToProlong.serialInfo, *(cdb.getItemDuration()));
				}

				if (!hasSufficientCurrency(accountInfo, totalCurrencySpentByType)) return;

				response.setData(reinterpret_cast<std::uint8_t*>(itemsToProlong.data()), itemsToProlong.size() * sizeof(Main::Structures::BoughtItemToProlong));
				session.asyncWrite(response);

				for (const auto& [currencyType, totalSpent] : totalCurrencySpentByType)
				{
					removeAmountByCurrencyType(currencyType, totalSpent, session, accountInfo);
				}
			}
			else // Buy new item
			{
				if (!session.hasEnoughInventorySpace(request.getOption())) return;

				std::uint64_t latestItemNumber = session.getLatestItemNumber();
				std::vector<Main::Structures::BoughtItem> boughtItems;

				for (std::uint32_t idx = 0; idx < request.getOption(); ++idx)
				{
					std::uint32_t itemId;
					std::memcpy(&itemId, request.getData() + idx * 4, sizeof(std::uint32_t));

					Main::Structures::BoughtItem boughtItem(itemId);
					const Main::ConstantDatabase::CdbUtil cdb(boughtItem.itemId);
					const auto amountByCurrencyType = cdb.getItemPrice();
					totalCurrencySpentByType[amountByCurrencyType->first] += amountByCurrencyType->second;
					boughtItem.serialInfo.itemNumber = ++latestItemNumber;
					boughtItems.push_back(boughtItem);
				}

				if (!hasSufficientCurrency(accountInfo, totalCurrencySpentByType)) return;

				response.setData(reinterpret_cast<std::uint8_t*>(boughtItems.data()), boughtItems.size() * sizeof(Main::Structures::BoughtItem));
				session.asyncWrite(response);

				session.setLatestItemNumber(latestItemNumber);
				session.addItems(boughtItems);

				for (const auto& [currencyType, totalSpent] : totalCurrencySpentByType)
				{
					removeAmountByCurrencyType(currencyType, totalSpent, session, accountInfo);
				}
			}
		}
	}
}

#endif
