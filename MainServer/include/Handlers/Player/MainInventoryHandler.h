#ifndef MAIN_ACCOUNT_INVENTORY_HANDLER_H
#define MAIN_ACCOUNT_INVENTORY_HANDLER_H

#include "Network/Session.h"
#include "../../MainEnums.h"
#include <unordered_map>
#include <Classes/Player.h>
#include <Network/MainSessionManager.h>

namespace Main
{
    namespace Handlers
    {
        inline void handleInventoryInformation(const Common::Network::Packet& request, Main::Network::Session& session,
            Main::Network::SessionsManager& sessionsManager, Main::Persistence::PersistentDatabase& database)
        {
            std::uint32_t accountID;
            std::memcpy(&accountID, request.getData(), sizeof(std::uint32_t));

            auto playerItems = database.getPlayerItems(accountID);
            auto& nonEquippedItems = playerItems.first;
            auto& equippedItems = playerItems.second;

            // Handle non-equipped items
            Common::Network::Packet response;
            response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
            response.setOrder(77);
            response.setOption(nonEquippedItems.size());

            constexpr std::size_t headerSize = sizeof(Common::Protocol::TcpHeader) + sizeof(Common::Protocol::CommandHeader);
            constexpr std::size_t MAX_PACKET_SIZE = 1440;
            std::size_t totalSize = headerSize + nonEquippedItems.size() * sizeof(Main::Structures::Item);

            if (totalSize == headerSize)
            {
                response.setData(nullptr, 0);
                response.setExtra(6);
            }
            else if (totalSize < MAX_PACKET_SIZE)
            {
                response.setExtra(37);
                response.setData(reinterpret_cast<std::uint8_t*>(nonEquippedItems.data()), nonEquippedItems.size() * sizeof(Main::Structures::Item));
            }
            else
            {
                std::size_t currentItemIndex = 0;
                const std::size_t maxPayloadSize = MAX_PACKET_SIZE - headerSize;
                const std::size_t itemsToSend = maxPayloadSize / sizeof(Main::Structures::Item);

                while (currentItemIndex < nonEquippedItems.size())
                {
                    auto endIndex = std::min(currentItemIndex + itemsToSend, nonEquippedItems.size());
                    response.setExtra(currentItemIndex == 0 ? 37 : 0);
                    response.setData(reinterpret_cast<std::uint8_t*>(nonEquippedItems.data() + currentItemIndex),
                        (endIndex - currentItemIndex) * sizeof(Main::Structures::Item));
                    response.setOption(endIndex - currentItemIndex);
                    session.asyncWrite(response);
                    currentItemIndex = endIndex;
                }
            }

            session.setUnequippedItems(nonEquippedItems);

            // Handle equipped items
            response.setOrder(75);
            response.setMission(0);

            for (const auto& [characterID, items] : equippedItems)
            {
                response.setExtra(characterID);
                response.setOption(items.size());
                response.setData(reinterpret_cast<std::uint8_t*>(const_cast<Main::Structures::EquippedItem*>(items.data())),
                    items.size() * sizeof(Main::Structures::EquippedItem));
                session.asyncWrite(response);
            }

            // End packet with extra=16 necessary
            response.setOption(0);
            response.setExtra(16);
            response.setData(nullptr, 0);
            session.asyncWrite(response);

            session.setEquippedItems(equippedItems);
        }
    }
}

#endif
