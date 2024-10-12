#ifndef CLIENT_REQUEST_STRUCTURES_H
#define CLIENT_REQUEST_STRUCTURES_H

#include <cstdint>
#include <Structures/Item/MainItemSerialInfo.h>

namespace Main
{
    namespace ClientData
    {
#pragma pack(push, 1)
        struct Ping
        {
            std::uint32_t unused{ 0 };  // Field reserved for future use, default to 0
            std::uint32_t ping{ 0 };    // Ping value (0-1023 range)
            std::uint32_t rest{ 0 };    // Reserved for future fields or usage

            // Set the ping value, ensuring it stays within the 10-bit range (0-1023)
            void setPing(std::uint32_t value) {
                ping = value & 0x3FF;   // Mask the value to keep only the lower 10 bits
            }
        };
#pragma pack(pop)

#pragma pack(push, 1)
        struct ItemRefund
        {
            Main::Structures::ItemSerialInfo serialInfo{};  // Contains serial info of the item
            std::uint32_t mpToAdd{};                        // Micro Points to refund
        };
#pragma pack(pop)

#pragma pack(push, 1)
        struct ItemUpgrade
        {
            Main::Structures::ItemSerialInfo serialInfo{};  // Contains serial info of the item
            std::uint32_t usedEnergy{};                     // Energy used for the upgrade
        };
#pragma pack(pop)

#pragma pack(push, 1)
        struct MailboxMessage
        {
            char nickname[16]{};    // Sender's nickname, maximum 16 characters
            char message[256]{};    // Message content, maximum 256 characters
        };
#pragma pack(pop)

#pragma pack(push, 1)
        struct RoomInfo
        {
            std::uint16_t roomNumber{};  // Room number
            std::uint16_t unknown{ 2 };  // Unknown field, usually set to 2
            char password[8]{};          // Room password, max 8 characters
        };
#pragma pack(pop)
    }
}

#endif
