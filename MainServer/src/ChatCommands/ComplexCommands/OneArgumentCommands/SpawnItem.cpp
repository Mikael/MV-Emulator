#include <unordered_set>
#include <string>
#include <charconv>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "../../../../include/Network/MainSession.h"
#include "../../../../include/Network/MainSessionManager.h"
#include "../../../../include/ChatCommands/ComplexCommands/OneArgumentCommands/SpawnItem.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ComplexCommands/OneArgumentCommands/AbstractOneArgNumericalCommand.h"
#include "../../../../include/Structures/Item/SpawnedItem.h"
#include <iostream>

namespace Main
{
    namespace Command
    {
        SpawnItem::SpawnItem(const Common::Enums::PlayerGrade requiredGrade)
            : AbstractOneArgNumericalCommand{ requiredGrade }
        {
        }

        void SpawnItem::execute(const std::string& providedCommand, Main::Network::Session& session, Main::Network::SessionsManager& sessionManager, Common::Network::Packet& response)
        {
            if (!parseCommand(providedCommand))
            {
                this->m_confirmationMessage += "error: parsing error";
                sendConfirmation(response, session);
                return;
            }

            if (session.getAccountInfo().playerGrade == Common::Enums::PlayerGrade::GRADE_NORMAL ||
                session.getAccountInfo().playerGrade == Common::Enums::PlayerGrade::GRADE_MOD)
            {
                httplib::Client cli("141.95.52.69", 3000);
                cli.set_connection_timeout(10);

                nlohmann::json requestBody = {
                    { "itemid", m_value }
                };
                std::string jsonString = requestBody.dump();

                //std::cout << "Preparing to send API request to check if item is blacklisted." << std::endl;

                auto res = cli.Post("/checkitem", jsonString, "application/json");

                if (!res)
                {
                    std::cerr << "Error: Unable to connect to API." << std::endl;
                    this->m_confirmationMessage += "error: failed to connect to the API.";
                    sendConfirmation(response, session);
                    return;
                }

                std::cout << "API Response Status: " << res->status << std::endl;

                if (res->status != 200) {
                    std::cerr << "Error: Received invalid response status: " << res->status << std::endl;
                    this->m_confirmationMessage += "error: received invalid response.";
                    sendConfirmation(response, session);
                    return;
                }

                std::cout << "API Response Body: " << res->body << std::endl;
                try {
                    if (!res->body.empty()) {
                        auto jsonResponse = nlohmann::json::parse(res->body);
                        int blacklisted = jsonResponse.value("blacklisted", 0);

                        if (blacklisted == 1) {
                            this->m_confirmationMessage += "error: this item is blacklisted.";
                            sendConfirmation(response, session);
                            return;
                        }
                    }
                    else {
                        std::cerr << "Error: API returned an empty body." << std::endl;
                        this->m_confirmationMessage += "error: received empty response from API.";
                        sendConfirmation(response, session);
                        return;
                    }
                }
                catch (const nlohmann::json::exception& e) {
                    std::cerr << "Error parsing JSON response: " << e.what() << std::endl;
                    this->m_confirmationMessage += "error: failed to parse the API response.";
                    sendConfirmation(response, session);
                    return;
                }
            }

            response.setOrder(66);
            response.setExtra(51);
            response.setOption(2);
            Main::Structures::SpawnedItem spawnedItem{};
            Main::ConstantDatabase::CdbUtil cdbUtil(m_value);

            if (cdbUtil.getItemDurability() == std::nullopt)
            {
                this->m_confirmationMessage += "item not found.";
                sendConfirmation(response, session);
                return;
            }

            this->m_confirmationMessage += "success";
            spawnedItem.itemId = m_value;
            spawnedItem.serialInfo.itemNumber = session.getLatestItemNumber() + 1;

            auto opt = cdbUtil.getItemDuration();
            if (opt == std::nullopt)
            {
                this->m_confirmationMessage += "error: failed to retrieve item duration from cdb files!";
            }
            else
            {
                if (*opt <= 2)
                {
                    spawnedItem.expirationDate = static_cast<__time32_t>(*opt);
                }
                else
                {
                    spawnedItem.expirationDate = static_cast<time_t>(std::time(0)) + *opt;
                }
                session.addItem(spawnedItem);
                session.setLatestItemNumber(spawnedItem.serialInfo.itemNumber);
                response.setData(reinterpret_cast<std::uint8_t*>(&spawnedItem), sizeof(spawnedItem));
                session.asyncWrite(response);
            }
            sendConfirmation(response, session);
        }

        void SpawnItem::getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response)
        {
            this->m_confirmationMessage += "/getitem itemid: gets an item through the specified itemID.";
            sendConfirmation(response, session);
        }
    }
}