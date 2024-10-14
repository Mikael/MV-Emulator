#ifndef CAST_SESSION_HEADER
#define CAST_SESSION_HEADER

#include "Network/Session.h"
#include <functional>
#include <chrono>
#include <vector>
#include <iostream>
#include "asio.hpp"
#include "../../../MainServer/include/Structures/AccountInfo/MainAccountInfo.h"

namespace Cast
{
    namespace Network
    {
        struct Session : public Common::Network::Session
        {
            Main::Structures::AccountInfo m_accountInfo{};
            Main::Structures::UniqueId m_uniqueId{};
            std::int64_t m_roomId{ -1 };
            bool m_isInMatch{ false };
            std::uint32_t m_roomNumber{};

            using tcp = asio::ip::tcp;

            explicit Session(tcp::socket&& socket, std::function<void(std::size_t)> fnct);

            void setRoomNumber(std::uint32_t roomNum);
            std::uint32_t getRoomNumber() const;
            void onPacket(std::vector<std::uint8_t>& data) override;
            void setAccountInfo(const Main::Structures::AccountInfo& accountInfo);
            void setRoomId(std::uint64_t id);
            std::uint64_t getRoomId() const;
            const Main::Structures::AccountInfo& getAccountInfo() const;
            void setUniqueId(const Main::Structures::UniqueId& uniqueId);
            const Main::Structures::UniqueId getUniqueId() const;
            bool isInMatch() const;
            void setIsInMatch(bool val);
            void setSessionId(std::size_t id) {
                m_id = id;
            }
            void updateMatchStatus(bool inMatch);
            void close();

        private:
            void notifyMatchStatusChange(bool inMatch);
        };
    }
}

#endif
