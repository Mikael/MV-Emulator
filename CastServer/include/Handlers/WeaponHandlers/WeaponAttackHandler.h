#ifndef WEAPON_HOST_ATTACK_HANDLER_H
#define WEAPON_HOST_ATTACK_HANDLER_H

#include "../../Network/CastSession.h"
#include "../../Classes/RoomsManager.h"
#include <unordered_map>
#include <iostream>
#include <chrono>

namespace Cast
{
    namespace Handlers
    {
        struct RateLimitData
        {
            int packetCount;
            int exceedCount; // Counter for exceeding rate limit
            std::chrono::steady_clock::time_point lastReset;
        };

        // Static map to track the packet rate of each session
        static std::unordered_map<int, RateLimitData> rateLimitMap;

        // Threshold for rate-limiting (adjust as necessary)
        const int MAX_PACKETS_PER_SECOND = 20;
        const auto RESET_INTERVAL = std::chrono::seconds(2); // Reset every second
        const int MAX_EXCEED_COUNT = 3; // Maximum times to exceed the limit before disconnecting

        // Helper function to check and update the rate limit for a session
        bool isRateLimited(int sessionId)
        {
            auto& data = rateLimitMap[sessionId];
            auto now = std::chrono::steady_clock::now();

            if (now - data.lastReset > RESET_INTERVAL)
            {
                data.packetCount = 0;
                data.exceedCount = 0; // Reset exceed count when the time window resets
                data.lastReset = now;
            }

            data.packetCount++;

            if (data.packetCount > MAX_PACKETS_PER_SECOND)
            {
                data.exceedCount++;
                return true; // Indicate that the rate limit has been exceeded
            }

            return false;
        }

        void handleHostWeaponAttack(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            int sessionId = session.getId();

            if (isRateLimited(sessionId))
            {
                auto& data = rateLimitMap[sessionId];
                if (data.exceedCount > MAX_EXCEED_COUNT)
                {
                    std::cerr << "Session " << sessionId << " disconnected for exceeding packet rate limit multiple times.\n";
                    session.close();  // Disconnect clients that exceed rate limits too many times
                    return;
                }
                std::cerr << "Session " << sessionId << " exceeded packet rate limit (count: " << data.exceedCount << ").\n";
            }

            try
            {
                roomsManager.broadcastToRoom(sessionId, const_cast<Common::Network::Packet&>(request));
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error handling packet for session " << sessionId << ": " << e.what() << '\n';
                session.close();
            }
        }

        inline void nonHostWeaponAttack(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            int sessionId = session.getId();

            if (isRateLimited(sessionId))
            {
                auto& data = rateLimitMap[sessionId];
                if (data.exceedCount > MAX_EXCEED_COUNT)
                {
                    std::cerr << "Session " << sessionId << " disconnected for exceeding packet rate limit multiple times.\n";
                    session.close();  // Disconnect clients that exceed rate limits too many times
                    return;
                }
                std::cerr << "Session " << sessionId << " exceeded packet rate limit (count: " << data.exceedCount << ").\n";
            }

            try
            {
                roomsManager.playerForwardToHost(request.getSession(), sessionId, const_cast<Common::Network::Packet&>(request));
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error forwarding packet for session " << sessionId << ": " << e.what() << '\n';
                session.close();  // Disconnect clients that encounter critical errors
            }
        }
    }
}

#endif
