#include "Network/Session.h"

#include <functional>
#include <chrono>
#include <vector>
#include <iostream>
#include "asio.hpp"
#include <Utils/Parser.h>
#include "../../../MainServer/include/Structures/AccountInfo/MainAccountInfo.h"
#include "../../include/Network/CastSession.h"
#include <Utils/Logger.h>

namespace Cast
{
    namespace Network
    {
        struct LogInfo {
            std::chrono::steady_clock::time_point lastLogged;
            int count;
        };

        Session::Session(tcp::socket&& socket, std::function<void(std::size_t)> fnct)
            : Common::Network::Session{ std::move(socket), fnct }
        {
        }

        void Session::onPacket(std::vector<std::uint8_t>& data)
        {
            Common::Network::Packet incomingPacket;
            incomingPacket.processIncomingPacket(data.data(), static_cast<std::uint16_t>(data.size()), std::nullopt);

            const std::uint16_t callbackNum = incomingPacket.getOrder();
            if (!Common::Network::Session::callbacks<Session>.contains(callbackNum))
            {
                std::cerr << "[CastSession] No callback for order: " << callbackNum << "\n";
                return;
            }

            Common::Network::Session::callbacks<Session>[callbackNum](incomingPacket, *this);
        }

        void Session::setAccountInfo(const Main::Structures::AccountInfo& accountInfo)
        {
            m_accountInfo = accountInfo;
        }

        void Session::setRoomId(std::uint64_t id)
        {
            m_roomId = id;
        }

        std::uint64_t Session::getRoomId() const
        {
            return m_roomId;
        }

        std::uint32_t Session::getRoomNumber() const
        {
            return m_roomNumber;
        }

        void Session::setRoomNumber(std::uint32_t roomNum)
        {
            m_roomNumber = roomNum;
        }

        const Main::Structures::AccountInfo& Session::getAccountInfo() const
        {
            return m_accountInfo;
        }

        void Session::setUniqueId(const Main::Structures::UniqueId& uniqueId)
        {
            m_uniqueId = uniqueId;
        }

        const Main::Structures::UniqueId Session::getUniqueId() const
        {
            return m_uniqueId;
        }

        bool Session::isInMatch() const
        {
            return m_isInMatch;
        }

        void Session::setIsInMatch(bool val)
        {
            std::cout << "Set Is In Match set to: " << val << " for sessionID: " << m_id << '\n';
            m_isInMatch = val;
        }

        void Session::close()
        {
            if (m_socket.is_open())
            {
                asio::error_code ec;
                m_socket.close(ec);
                if (ec)
                {
                    std::cerr << "Error closing session socket: " << ec.message() << "\n";
                }
                else
                {
                    std::cout << "Session closed successfully.\n";
                }
            }
        }
    };
}
