
#include "../../include/Classes/Player.h"

#include <unordered_map>
#include <vector>

#include "../../include/Network/MainSession.h"
#include <ConstantDatabase/Structures/SetItemInfo.h>
#include <Utils/Utils.h>

namespace Main
{
	namespace Classes
	{
		using Item = Main::Structures::Item;
		using EquippedItem = Main::Structures::EquippedItem;
		using DetailedEquippedItem = Main::Structures::DetailedEquippedItem;
		using BoughtItem = Main::Structures::BoughtItem;
		using AccountInfo = Main::Structures::AccountInfo;
		using Session = Main::Network::Session;
		using BlockedPlayer = Main::Structures::BlockedPlayer;
		using Friend = Main::Structures::Friend;
		using TradedItem = Main::Structures::TradeBasicItem;
		using Mailbox = Main::Structures::Mailbox;
		using Session = Main::Network::Session;

		Player::Player()
		{
			for (std::uint64_t characterID = 0; characterID <= 8; ++characterID) {
				m_equippedItemByCharacter.emplace(characterID, std::unordered_map<std::uint64_t, EquippedItem>{});
			}
		}
		void Player::setPing(std::uint16_t ping)
		{
			m_ping = ping;
		}

		std::uint16_t Player::getPing() const
		{
			return m_ping;
		}
		void Player::setAccountInfo(const AccountInfo& accountInfo)
		{
			m_accountInfo = accountInfo;
		}

		void Player::addBatteryObtainedInMatch(std::uint32_t newBattery)
		{
			m_batteryObtainedInMatch += newBattery;
		}

		void Player::storeBatteryObtainedInMatch()
		{
			std::cout << "Battery Obtained: " << m_batteryObtainedInMatch << '\n';

			if (m_accountInfo.battery + m_batteryObtainedInMatch >= m_accountInfo.maxBattery)
			{
				std::cout << "Max Battery: " << m_accountInfo.maxBattery;
				m_accountInfo.battery = m_accountInfo.maxBattery;
			}
			else
			{
				m_accountInfo.battery += m_batteryObtainedInMatch;
			}
		}

		const AccountInfo& Player::getAccountInfo() const
		{
			return m_accountInfo;
		}

		std::uint32_t Player::getAccountID() const
		{
			return m_accountInfo.accountID;
		}

		const char* const Player::getPlayerName() const
		{
			return m_accountInfo.nickname;
		}

		void Player::setAccountRockTotens(std::uint32_t rt)
		{
			m_accountInfo.rockTotens = rt;
		}

		void Player::setAccountMicroPoints(std::uint32_t mp)
		{
			m_accountInfo.microPoints = mp;
		}

		void Player::setAccountCoins(std::uint16_t coins)
		{
			m_accountInfo.coins = coins;
		}

		void Player::setAccountLatestCharacterSelected(std::uint16_t latestCharacterSelected)
		{
			m_accountInfo.latestSelectedCharacter = latestCharacterSelected;
		}

		void Player::setLevel(std::uint16_t level)
		{
			m_accountInfo.playerLevel = level + 1;
		}

		void Player::setPlayerName(const char* playerName)
		{
			strcpy_s(m_accountInfo.nickname, playerName);
		}

		bool Player::hasEnoughInventorySpace(std::uint16_t totalNewItems) const
		{
			return (static_cast<std::int32_t>(m_accountInfo.inventorySpace) - m_totalEquippedItems - m_itemsByItemNumber.size()) > totalNewItems;
		}

		void Player::setPlayerState(Common::Enums::PlayerState playerState)
		{
			m_playerState = playerState;
		}

		Common::Enums::PlayerState Player::getPlayerState() const
		{
			return m_playerState;
		}

		void Player::setIsInLobby(bool val)
		{
			m_isInLobby = val;
		}

		bool Player::isInLobby() const
		{
			return m_isInLobby;
		}

		Main::Structures::MuteInfo Player::getMuteInfo() const
		{
			return Main::Structures::MuteInfo{ m_isMuted, m_muteReason, m_mutedBy, m_mutedUntil };
		}

		void Player::mute(const std::string& reason, const std::string& mutedBy, const std::string& mutedUntil)
		{
			m_isMuted = true;
			m_muteReason = reason;
			m_mutedBy = mutedBy;
			m_mutedUntil = mutedUntil;
		}

		void Player::unmute()
		{
			m_isMuted = false;
		}

		bool Player::isMuted() const
		{
			return m_isMuted;
		}

		void Player::addLuckyPoints(std::uint32_t points)
		{
			m_accountInfo.luckyPoints += points;
		}

		void Player::setLuckyPoints(std::uint32_t points)
		{
			m_accountInfo.luckyPoints = points;
		}

		std::uint32_t Player::getLuckyPoints() const
		{
			return static_cast<std::uint32_t>(m_accountInfo.luckyPoints);
		}

		const std::vector<Friend> Player::getFriendlist() const
		{
			std::vector<Friend> ret;
			for (const auto& [ffriend, session] : m_friends)
			{
				ret.push_back(ffriend);
			}
			return ret;
		}

		std::unordered_map<Friend, Session*>& Player::getFriendSessions()
		{
			return m_friends;
		}

		void Player::setFriendList(const std::vector<Friend>& friendlist)
		{
			for (const auto& currentFriend : friendlist)
			{
				m_friends[currentFriend] = nullptr;
			}
		}

		void Player::updateFriend(const Friend& targetFriend, Session* targetSession, bool remove = false)
		{
			if (m_friends.contains(targetFriend)) m_friends.erase(targetFriend);
			m_friends[targetFriend] = remove ? nullptr : targetSession;
		}

		// call once with default "persist", since removeFriend removes the friend for both players
		void Player::deleteFriend(std::uint32_t targetAccountId)
		{
			Main::Structures::Friend toDelete;
			toDelete.targetAccountId = targetAccountId;
			m_friends.erase(toDelete);
		}

		void Player::addOfflineFriend(const Main::Structures::Friend& ffriend)
		{
			m_friends[ffriend] = nullptr;
		}

		std::optional<Main::Structures::Friend> Player::addOnlineFriend(Session* session)
		{
			if (session)
			{
				const auto& accountInfo = session->getAccountInfo();
				Main::Structures::Friend ffriend{ accountInfo.uniqueId, accountInfo.accountID };
				std::memcpy(ffriend.targetNickname, accountInfo.nickname, 16);
				m_friends[ffriend] = session;
				return ffriend;
			}
			return std::nullopt;
		}

		bool Player::isFriend(std::uint32_t accountId)
		{
			for (const auto& [key, val] : m_friends)
			{
				if (key.targetAccountId == accountId)
				{
					return true;
				}
			}
			return false;
		}

		void Player::setUnequippedItems(const std::vector<Item>& items)
		{
			for (const auto& currentItem : items)
			{
				m_latestItemNumber = std::max(m_latestItemNumber, currentItem.serialInfo.itemNumber);
				m_itemsByItemNumber[currentItem.serialInfo.itemNumber] = currentItem;
			}
		}

		const std::optional<Item> Player::findItemBySerialInfo(const Main::Structures::ItemSerialInfo& itemSerialInfo) const
		{
			for (const auto& [itemNumber, item] : m_itemsByItemNumber)
			{
				if (item.serialInfo == itemSerialInfo)
				{
					return item;
				}
			}
			for (const auto& [itemType, item] : m_equippedItemByCharacter.at(m_accountInfo.latestSelectedCharacter)) // m_equippedItemByType)
			{
				if (item.serialInfo == itemSerialInfo)
				{
					return Item{ item };
				}
			}
			return std::nullopt;
		}

		bool Player::replaceItem(const std::uint32_t itemNum, const Main::Structures::ItemSerialInfo& newItemSerialInfo, std::uint64_t newExpiration)
		{
			auto itemIt = m_itemsByItemNumber.find(itemNum);
			if (itemIt != m_itemsByItemNumber.end())
			{
				itemIt->second.serialInfo = newItemSerialInfo;
				return true;
			}

			auto characterIt = m_equippedItemByCharacter.find(m_accountInfo.latestSelectedCharacter);
			if (characterIt != m_equippedItemByCharacter.end())
			{
				for (auto& [itemType, item] : characterIt->second)
				{
					if (item.serialInfo.itemNumber == itemNum)
					{
						item.serialInfo = newItemSerialInfo; 
						return true; 
					}
				}
			}

			return false;
		}


		const std::unordered_map<std::uint64_t, EquippedItem>& Player::getEquippedItems() const
		{
			return m_equippedItemByCharacter.at(m_accountInfo.latestSelectedCharacter);
		}

		const std::unordered_map<std::uint64_t, Item>& Player::getItems() const
		{
			return m_itemsByItemNumber;
		}

		const std::vector<Item> Player::getItemsAsVec() const
		{
			std::vector<Item> items;
			items.reserve(m_itemsByItemNumber.size());
			for (const auto& [unused, item] : m_itemsByItemNumber)
			{
				items.push_back(item);
			}
			return items;
		}

		bool Player::deleteItemBasic(const Main::Structures::ItemSerialInfo& itemSerialInfo)
		{
			for (auto it = m_itemsByItemNumber.begin(); it != m_itemsByItemNumber.end(); ++it)
			{
				if (it->second.serialInfo == itemSerialInfo)
				{
					m_itemsByItemNumber.erase(it);
					return true;
				}
			}
			return false;
		}

		void Player::addItems(const std::vector<Item>& items)
		{
			for (auto& currentItem : items)
			{
				m_itemsByItemNumber[currentItem.serialInfo.itemNumber] = currentItem;
			}
		}

		void Player::addItem(const Item& item)
		{
			m_itemsByItemNumber[item.serialInfo.itemNumber] = item;
		}

		std::vector<Item> Player::addItems(const std::vector<BoughtItem>& boughtItems)
		{
			std::vector<Item> items;
			for (auto& currentItem : boughtItems)
			{
				items.push_back(currentItem);
				m_itemsByItemNumber[currentItem.serialInfo.itemNumber] = currentItem;
			}
			return items;
		}

		std::vector<Main::Structures::Item> Player::addItems(const std::vector<Main::Structures::TradeBasicItem>& tradedItems)
		{
			std::vector<Item> items;
			for (auto& currentItem : tradedItems)
			{
				items.push_back(currentItem);
				m_itemsByItemNumber[currentItem.itemSerialInfo.itemNumber] = currentItem;
			}
			return items;
		}

		Item Player::addItemFromTrade(TradedItem tradeItem)
		{
			tradeItem.itemSerialInfo.itemNumber = ++m_latestItemNumber;
			Main::Structures::Item item{ tradeItem };
			m_itemsByItemNumber[tradeItem.itemSerialInfo.itemNumber] = item;
			return item;
		}

		void Player::setEquippedItems(const std::unordered_map<std::uint16_t, std::vector<EquippedItem>>& equippedItems)
		{
			for (const auto& [characterID, items] : equippedItems)
			{
				for (const auto& currentItem : items)
				{
					m_latestItemNumber = std::max(m_latestItemNumber, currentItem.serialInfo.itemNumber);
					auto& itemMap = m_equippedItemByCharacter[characterID];
					itemMap[currentItem.type >= 17 ? Common::Enums::SET : currentItem.type] = currentItem;
					++m_totalEquippedItems;
				}
			}
		}

		std::optional<std::pair<std::uint16_t, std::uint64_t>>
			Player::addEnergyToItem(const Main::Structures::ItemSerialInfo& itemSerialInfo, std::uint32_t energyAdded)
		{
			for (auto& [itemNumber, item] : m_itemsByItemNumber)
			{
				if (item.serialInfo == itemSerialInfo)
				{
					item.energy += energyAdded;
					m_accountInfo.battery -= energyAdded;
					return std::pair{ item.energy, m_accountInfo.battery };
				}
			}
			for (auto& [itemType, item] : m_equippedItemByCharacter.at(m_accountInfo.latestSelectedCharacter))
			{
				if (item.serialInfo == itemSerialInfo)
				{
					item.energy += energyAdded;
					m_accountInfo.battery -= energyAdded;
					return std::pair{ item.energy, m_accountInfo.battery };
				}
			}
			return std::nullopt;
		}

		void Player::unequipItemImpl(std::uint64_t itemType, Main::Persistence::MainScheduler& scheduler)
		{
			auto& itemMap = m_equippedItemByCharacter[m_accountInfo.latestSelectedCharacter];
			if (!itemMap.contains(itemType)) return;

			auto itemNumber = itemMap.at(itemType).serialInfo.itemNumber;
			m_itemsByItemNumber[itemNumber] = Item{ itemMap.at(itemType) };
			itemMap.erase(itemType);
			--m_totalEquippedItems;

			std::cout << "Item Unequipped with Type: " << itemType << '\n';

			scheduler.addRepetitiveCallback(m_accountInfo.accountID, &Main::Persistence::PersistentDatabase::unequipItem,
				m_accountInfo.accountID, static_cast<std::uint64_t>(itemNumber));
		}

		void Player::equipItem(const std::uint16_t itemNumber, Main::Persistence::MainScheduler& scheduler)
		{
			if (m_itemsByItemNumber.find(itemNumber) == m_itemsByItemNumber.end())
			{
				std::cout << "Item Number " << itemNumber << " does not exist.\n";
				return;
			}

			EquippedItem equippedItem = EquippedItem{ m_itemsByItemNumber[itemNumber] };

			if (equippedItem.type == Common::Enums::ItemType::SET)
			{
				std::cout << "User Equipping Set. Unequipping Parts Relative To Set...\n";
				using setItems = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::SetItemInfo>;
				const auto entry = setItems::getInstance().getEntry("si_id", m_itemsByItemNumber[itemNumber].id);

				if (entry)
				{
					for (auto currentTypeNotNull : Common::Utils::getPartTypesWhereSetItemInfoTypeNotNull(*entry))
					{
						std::cout << "Set Part Found. Unequipping (Type: " << currentTypeNotNull << ")\n";
						unequipItemImpl(currentTypeNotNull, scheduler);
					}
				}
			}

			auto& itemMap = m_equippedItemByCharacter[m_accountInfo.latestSelectedCharacter];

			auto equippedSetIt = itemMap.find(Common::Enums::ItemType::SET);
			if (equippedSetIt != itemMap.end())
			{
				std::cout << "User has a Set Equipped. Unequipping the set before equipping the parts...\n";
				using setItems = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::SetItemInfo>;
				const auto entry = setItems::getInstance().getEntry("si_id", equippedSetIt->second.id >> 1);

				if (entry)
				{
					for (auto currentTypeNotNull : Common::Utils::getPartTypesWhereSetItemInfoTypeNotNull(*entry))
					{
						if (equippedItem.type == currentTypeNotNull)
						{
							std::cout << "Set Found. Unequipping...\n";
							unequipItemImpl(Common::Enums::ItemType::SET, scheduler);
							break;
						}
					}
				}
			}

			m_itemsByItemNumber.erase(itemNumber);

			auto equippedIt = itemMap.find(equippedItem.type);
			if (equippedIt != itemMap.end())
			{
				std::cout << "Unequipping Item...\n";
				auto toUnequipItemNumber = equippedIt->second.serialInfo.itemNumber;

				m_itemsByItemNumber[toUnequipItemNumber] = Item{ equippedIt->second };
				itemMap.erase(equippedIt); 
			}

			itemMap[equippedItem.type] = equippedItem;
			++m_totalEquippedItems;

			scheduler.addRepetitiveCallback(m_accountInfo.accountID, &Main::Persistence::PersistentDatabase::equipItem,
				m_accountInfo.accountID, static_cast<std::uint64_t>(equippedItem.serialInfo.itemNumber), static_cast<std::uint16_t>(m_accountInfo.latestSelectedCharacter));
		}


		std::optional<std::uint64_t> Player::unequipItem(std::uint64_t itemType, Main::Persistence::MainScheduler& scheduler)
		{
			auto& itemMap = m_equippedItemByCharacter[m_accountInfo.latestSelectedCharacter];

			auto it = itemMap.find(itemType);
			if (it == itemMap.end())
			{
				std::cout << "Item NOT unequipped!\n";
				return std::nullopt;
			}

			std::uint64_t itemNumber = it->second.serialInfo.itemNumber;
			m_itemsByItemNumber[itemNumber] = Item{ it->second };

			itemMap.erase(it);
			--m_totalEquippedItems;

			std::cout << "Item Unequipped\n";
			return itemNumber;
		}


		std::uint64_t Player::getTotalEquippedItems() const
		{
			return m_totalEquippedItems;
		}

		std::uint64_t Player::getLatestItemNumber() const
		{
			return m_latestItemNumber;
		}

		void Player::setLatestItemNumber(std::uint64_t itemNum)
		{
			m_latestItemNumber = itemNum;
		}

		std::pair<std::array<std::uint32_t, 10>, std::array<std::uint32_t, 7>> Player::getEquippedItemsSeparated() const
		{
			std::array<std::uint32_t, 10> equippedPlayerItems{};
			std::array<std::uint32_t, 7> equippedPlayerWeapons{};

			const auto& itemMap = m_equippedItemByCharacter.at(m_accountInfo.latestSelectedCharacter);

			if (auto setItemIt = itemMap.find(Common::Enums::SET); setItemIt != itemMap.end())
			{
				using setItems = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::SetItemInfo>;
				const auto setItemId = setItemIt->second.id >> 1;

				if (auto entry = setItems::getInstance().getEntry("si_id", setItemId); entry != std::nullopt)
				{
					for (const auto& currentTypeNotNull : Common::Utils::getPartTypesWhereSetItemInfoTypeNotNull(*entry))
					{
						equippedPlayerItems[currentTypeNotNull] = setItemId;
					}
				}
			}

			for (std::size_t i = 0; i < equippedPlayerItems.size(); ++i)
			{
				if (auto it = itemMap.find(i); it != itemMap.end())
				{
					equippedPlayerItems[i] = it->second.id >> 1;
				}
			}

			for (std::size_t i = 0; i < equippedPlayerWeapons.size(); ++i)
			{
				if (auto it = itemMap.find(i + 10); it != itemMap.end())
				{
					equippedPlayerWeapons[i] = it->second.id >> 1;
				}
			}

			return { equippedPlayerItems, equippedPlayerWeapons };
		}


		void Player::blockAccount(std::uint32_t accountId, const char* nickname)
		{
			Main::Structures::BlockedPlayer blocked{ accountId };
			std::memcpy(blocked.targetNickname, nickname, 16);

			m_blockedAccounts.push_back(blocked);
		}

		bool Player::unblockAccount(std::uint32_t accountId)
		{
			for (auto it = m_blockedAccounts.begin(); it != m_blockedAccounts.end(); ++it)
			{
				if (it->targetAccountId == accountId)
				{
					m_blockedAccounts.erase(it);
					return true;
				}
			}
			return false;
		}

		bool Player::hasBlocked(std::uint32_t accountId) const
		{
			for (const auto& currentBlocked : m_blockedAccounts)
			{
				if (currentBlocked.targetAccountId == accountId)
				{
					return true;
				}
			}
			return false;
		}

		const std::vector<Main::Structures::BlockedPlayer>& Player::getBlockedPlayers() const
		{
			return m_blockedAccounts;
		}

		void Player::setBlockedPlayers(const std::vector<Main::Structures::BlockedPlayer>& blockedPlayers)
		{
			m_blockedAccounts = blockedPlayers;
		}


		// Trade system
		void Player::lockTrade()
		{
			m_hasPlayerLocked = true;
		}

		bool Player::hasPlayerLocked() const
		{
			return m_hasPlayerLocked;
		}

		void Player::resetTradeInfo()
		{
			m_hasPlayerLocked = false;
			m_tradedItems.clear();
			m_currentlyTradingWithAccountId = 0;
			setPlayerState(Common::Enums::PlayerState::STATE_INVENTORY);
		}

		void Player::setCurrentlyTradingWithAccountId(std::uint32_t targetAccountId)
		{
			m_currentlyTradingWithAccountId = targetAccountId;
		}

		std::uint32_t Player::getCurrentlyTradingWithAccountId() const
		{
			return m_currentlyTradingWithAccountId;
		}

		void Player::addTradedItem(std::uint32_t itemId, const Main::Structures::ItemSerialInfo& serialInfo)
		{
			m_tradedItems.push_back(Main::Structures::TradeBasicItem{ itemId , serialInfo });
		}

		void Player::removeTradedItem(const Main::Structures::ItemSerialInfo& serialInfo)
		{
			for (auto it = m_tradedItems.begin(); it != m_tradedItems.end(); ++it)
			{
				if (it->itemSerialInfo == serialInfo)
				{
					m_tradedItems.erase(it);
					return;
				}
			}
		}

		void Player::resetTradedItems()
		{
			m_tradedItems.clear();
		}

		const std::vector<TradedItem>& Player::getTradedItems() const
		{
			return m_tradedItems;
		}


		// Mailbox
		void Player::addMailboxReceived(const Main::Structures::Mailbox& mailbox)
		{
			m_mailboxReceived.push_back(mailbox);
		}

		void Player::addMailboxSent(const Main::Structures::Mailbox& mailbox)
		{
			m_mailboxSent.push_back(mailbox);
		}

		bool Player::deleteSentMailbox(std::uint32_t timestamp, std::uint32_t accountId)
		{
			for (auto it = m_mailboxSent.begin(); it != m_mailboxSent.end(); ++it)
			{
				if (it->accountId == accountId && it->timestamp == timestamp)
				{
					m_mailboxSent.erase(it);
					return true;
				}
			}
			return false;
		}

		bool Player::deleteReceivedMailbox(std::uint32_t timestamp, std::uint32_t accountId)
		{
			for (auto it = m_mailboxReceived.begin(); it != m_mailboxReceived.end(); ++it)
			{
				if (it->accountId == accountId && it->timestamp == timestamp)
				{
					m_mailboxReceived.erase(it);
					return true;
				}
			}
			return false;
		}

		const std::vector<Main::Structures::Mailbox>& Player::getMailboxReceived() const
		{
			return m_mailboxReceived;
		}

		const std::vector<Main::Structures::Mailbox>& Player::getMailboxSent() const
		{
			return m_mailboxSent;
		}

		void Player::setMailbox(const std::vector<Main::Structures::Mailbox>& mailbox, bool sent)
		{
			if (sent) m_mailboxSent = mailbox;
			else m_mailboxReceived = mailbox;
		}


		// Room info
		void Player::setRoomNumber(std::uint16_t roomNumber)
		{
			m_roomNumber = roomNumber;
		}

		std::uint16_t Player::getRoomNumber() const
		{
			return m_roomNumber;
		}

		void Player::setIsInMatch(bool val)
		{
			m_isInMatch = val;
		}

		bool Player::isInMatch() const
		{
			return m_isInMatch;
		}

		void Player::leaveRoom()
		{
			setRoomNumber(0);
			setIsInMatch(false);
			m_batteryObtainedInMatch = 0;
		}

		void Player::decreaseRoomNumber()
		{
			if (m_roomNumber > 0)
			{
				--m_roomNumber;
			}
		}
	}
}
