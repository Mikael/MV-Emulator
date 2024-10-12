#ifndef AUTH_INITIAL_PACKET_STRUCTURE_H
#define AUTH_INITIAL_PACKET_STRUCTURE_H

#ifdef WIN32
#include <corecrt.h>
#else
#define __time32_t uint32_t
#endif

#include <ctime>
#include <httplib.h> // Use httplib for HTTP requests
#include <nlohmann/json.hpp> // Include JSON library
#include <iostream>

#include "Protocol/TcpHeader.h"
#include "Protocol/CommandHeader.h"
#include "Enums/MiscellaneousEnums.h"
#include "Enums/ExtrasEnums.h"

namespace Auth
{
    namespace Structures
    {
#pragma pack(push, 1)
        struct ConnectionPacket
        {
            Common::Protocol::TcpHeader tcpHeader{};
            Common::Protocol::CommandHeader commandHeader{};
            std::int32_t key{};
            __time32_t timestamp{};

            ConnectionPacket()
                : key{ static_cast<std::int32_t>(rand() + 1) }
                , timestamp{ static_cast<__time32_t>(std::time(0)) }
            {
                tcpHeader.initialize(0, Common::Enums::EncryptionType::NO_ENCRYPTION, sizeof(ConnectionPacket));

                // Check if the server is in maintenance mode
                bool isMaintenance = checkMaintenanceStatus();

                // Set the command header based on the maintenance status
                if (isMaintenance)
                {
                    commandHeader.initialize(401, Common::Random::generateRandomOption(), static_cast<int>(Common::Enums::AUTH_MAINTENANCE), Common::Random::generateRandomMission());
                }
                else
                {
                    commandHeader.initialize(401, Common::Random::generateRandomOption(), static_cast<int>(Common::Enums::AUTH_SUCCESS), Common::Random::generateRandomMission());
                }
            }

            // Function to check maintenance status from the API
// Function to check maintenance status from the API
            bool checkMaintenanceStatus()
            {
                // Use specific IP and port here
                httplib::Client cli("141.95.52.69", 3000); // Initialize the client with IP and port
                cli.set_connection_timeout(2); // Set connection timeout (optional)

                // Send a GET request to the maintenance-status endpoint
                std::cout << "Sending request to maintenance status endpoint..." << std::endl; // Debug output
                auto res = cli.Get("/maintenance-status");

                if (!res)
                {
                    std::cerr << "Error: Unable to connect to API." << std::endl;
                    return false;
                }

                std::cout << "API Response Status: " << res->status << std::endl;

                // If the request was successful, check the response
                if (res->status == 200)
                {
                    try
                    {
                        // Output the entire body of the response
                        std::cout << "API Response Body: " << res->body << std::endl; // Print the body of the response

                        // Parse the JSON response
                        auto jsonResponse = nlohmann::json::parse(res->body);
                        bool maintenance = jsonResponse.value("maintenance", false); // Get the 'maintenance' field from the JSON response
                        return maintenance;
                    }
                    catch (const nlohmann::json::exception& e)
                    {
                        std::cerr << "Error parsing JSON response: " << e.what() << std::endl;
                        return false;
                    }
                }
                else
                {
                    std::cerr << "Error: Received invalid response status: " << res->status << std::endl;
                }

                return false;
            }

        };
#pragma pack(pop)
    }
}

#endif
