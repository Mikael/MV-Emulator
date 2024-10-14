#include <vector>
#include <iostream>
#include <cstring>
#include <ctime>
#include <optional>
#include <mutex>
#include <atomic> // Include for std::atomic

#ifdef WIN32
#include <corecrt.h>
#else
#define __time32_t uint32_t
#endif

#include "../../include/Network/Session.h"
#include "../../include/Utils/Parser.h"
#include "../../include/Enums/ExtrasEnums.h"
#include "../../../MainServer/include/Structures/AccountInfo/MainAccountUniqueId.h"

namespace Common {
    namespace Network {

        // Static members for session ID management
        static std::atomic<std::uint32_t> nextSessionId{ 1 }; // Atomic session ID
        static std::mutex sessionIdMutex; // Optional if you use atomic

        Session::Session(asio::io_context& io_context)
            : m_socket(io_context) // Initialize the socket with io_context
        {
            m_id = nextSessionId.fetch_add(1); // Atomically assign a unique session ID
        }

        void Session::asyncWrite(const Common::Network::Packet& message)
        {
            std::unique_lock lock(m_sendMutex);
            std::optional<std::uint32_t> key = m_crypt.isUsed ? std::make_optional(m_crypt.UserKey) : std::nullopt;
            std::vector<uint8_t> encryptedMessage = message.generateOutgoingPacket(key);
            m_sendQueue.push(encryptedMessage);
            lock.unlock();

            if (key == std::nullopt) {
                // Common::Parser::parse_cast(encryptedMessage.data(), message.getFullSize(), 13006, "server","client");
            }
            else {
                // Common::Parser::parse(encryptedMessage.data(), message.getFullSize(), 13000, "server", "client", m_crypt.UserKey);
            }

            bool expected = false, desired = true;
            if (m_isInSend.compare_exchange_strong(expected, desired))
            {
                write();
            }
        }

        void Session::asyncRead() {
            if (!m_socket.is_open()) {
                std::printf("Socket is not open");
                return;
            }

            m_socket.async_read_some(asio::buffer(m_buffer.data(), m_buffer.size()),
                [self = this->shared_from_this()](const asio::error_code& error, std::size_t bytes_transferred) {
                    if (error) {
                        std::printf("asyncRead Error: %s\n", error.message().c_str());
                        self->closeSocket(); // Close the socket on error
                        return; // Do not attempt to read again
                    }

                    self->onRead(error, bytes_transferred);
                    self->asyncRead(); // Continue reading
                });
        }

        void Session::write()
        {
            std::unique_lock lock(m_sendMutex);
            if (m_sendQueue.empty())
            {
                m_isInSend = false;
                return;
            }
            if (!m_socket.is_open())
            {
                return;
            }
            auto& nextMessage = m_sendQueue.front();
            lock.unlock();

            asio::async_write(m_socket, asio::buffer(nextMessage.data(), nextMessage.size()),
                [&, self = this->shared_from_this()](const asio::error_code& errorCode, std::size_t)
                {
                    if (!errorCode)
                    {
                        std::unique_lock lock(m_sendMutex);
                        self->m_sendQueue.pop();
                        lock.unlock();
                        self->write();
                    }
                    else
                    {
                        self->closeSocket();
                        std::printf("Failed to send server message: %s\n", errorCode.message().c_str());
                    }
                });
        }

        void Session::onRead(asio::error_code error, std::size_t bytes_transferred) {
            // Check for error in reading
            if (error) {
                std::printf("onRead() Error: %s\n", error.message().c_str());
                closeSocket();
                return;
            }

            const constexpr int headerSize = sizeof(Common::Protocol::TcpHeader);
            // Add the newly received bytes to the reader buffer
            m_reader.insert(m_reader.end(), m_buffer.begin(), m_buffer.begin() + bytes_transferred);

            Common::Protocol::TcpHeader header;
            Common::Cryptography::Crypt cryptography(0);

            // Process packets in the reader buffer
            while (m_reader.size() >= headerSize) {
                // Decrypt or copy the header based on the encryption state
                if (m_crypt.isUsed) {
                    cryptography.RC5Decrypt32(reinterpret_cast<int32_t*>(m_reader.data()), &header, headerSize);
                }
                else {
                    std::memcpy(&header, m_reader.data(), headerSize);
                }

                // Validate packet size
                if (header.getSize() >= 2047) {
                    std::printf("Session::onRead() - Invalid packet size: %d\n", header.getSize());
                    closeSocket();
                    return;
                }

                // Ensure the entire packet is available in the buffer
                if (m_reader.size() >= static_cast<std::size_t>(header.getSize())) {
                    // Extract the complete packet
                    std::vector<std::uint8_t> data(m_reader.begin(), m_reader.begin() + header.getSize());
                    onPacket(data);  // Handle the complete packet

                    // Remove the processed packet from the reader buffer
                    m_reader.erase(m_reader.begin(), m_reader.begin() + header.getSize());
                }
                else {
                    // Exit the loop if there's not enough data for a complete packet
                    break;
                }
            }
        }

        std::uint8_t* Session::encryptRawMessage(std::uint8_t* data, std::size_t size) {
            constexpr std::size_t headerSize = sizeof(Common::Protocol::TcpHeader);
            m_defaultCrypt.RC5Encrypt64(data, data, headerSize);
            m_crypt.RC6Encrypt128(data + headerSize, data + headerSize, static_cast<int>(size - headerSize));
            return data;
        }

        void Session::closeSocket() {
            std::cout << "m_id: " << m_id << '\n';
            m_onCloseSocketCallback(m_id);
            if (!m_socket.is_open()) {
                return;
            }
            asio::error_code errorCode;
            auto endPoint = m_socket.remote_endpoint(errorCode);
            std::printf("Session::closeSocket() - Connection closed from %s:%d\n", endPoint.address().to_string().c_str(), endPoint.port());
            m_socket.shutdown(tcp::socket::shutdown_both, errorCode);
            m_socket.close(errorCode);
        }

        std::uint8_t* Session::getBufferData() {
            return m_buffer.data();
        }

        std::size_t Session::getBufferSize() const {
            return m_buffer.size();
        }

        Common::Cryptography::Crypt Session::getUserCrypt() const {
            return m_crypt;
        }

        Common::Cryptography::Crypt Session::getDefaultCrypt() const {
            return m_defaultCrypt;
        }

        void Session::sendConnectionACK(Common::Enums::ServerType serverType) {
            Packet connectionAck;
            connectionAck.setTcpHeader(m_id, Common::Enums::EncryptionType::NO_ENCRYPTION);

            switch (serverType) {
            case Enums::AUTH_SERVER: {
                struct AuthAck
                {
                    std::int32_t key{ static_cast<std::int32_t>(rand() + 1) };
                    __time32_t timestamp{ static_cast<__time32_t>(std::time(0)) };
                } authAck;

                m_crypt.KeySetup(authAck.key);
                connectionAck.setData(reinterpret_cast<std::uint8_t*>(&authAck), sizeof(AuthAck));
                connectionAck.setCommand(401, 0, static_cast<int>(Common::Enums::AUTH_SUCCESS), 0);
                asyncWrite(connectionAck);
                break;
            }

            case Enums::MAIN_SERVER: {
                struct UniqueId {
                    std::uint32_t session : 16 = 0;
                    std::uint32_t server : 15 = 4;
                    std::uint32_t unknown : 1 = 0;
                };

                struct MainAck
                {
                    std::int32_t key{ static_cast<std::int32_t>(rand() + 1) };
                    UniqueId uniqueId{};
                } mainAck;

                mainAck.uniqueId.session = m_id;
                mainAck.uniqueId.server = 4;
                std::cout << "MainAck uniqueId.session: " << mainAck.uniqueId.session << std::endl;

                m_crypt.KeySetup(mainAck.key);
                connectionAck.setData(reinterpret_cast<std::uint8_t*>(&mainAck), sizeof(MainAck));
                connectionAck.setCommand(401, 0, static_cast<int>(Common::Enums::MAIN_SUCCESS), 1);
                asyncWrite(connectionAck);
                break;
            }

            case Enums::CAST_SERVER:
            {
                m_crypt.isUsed = false;
                struct CastAck
                {
                    std::int32_t key{ static_cast<std::int32_t>(rand() + 1) };
                } castAck;
                connectionAck.setData(reinterpret_cast<std::uint8_t*>(&castAck), sizeof(castAck));
                connectionAck.setCommand(401, 0, Common::Enums::CAST_SUCCESS, 0);
                asyncWrite(connectionAck);
                break;
            }
            }

            asyncRead();
        }

        std::size_t Session::getId() const
        {
            return m_id;
        }

    } // namespace Network
} // namespace Common
