#ifndef CLIENT_REQUEST_STRUCTURES_H
#define CLIENT_REQUEST_STRUCTURES_H

#include <cstdint>

namespace Main
{
	namespace ClientData
	{
#pragma pack(push, 1)
		struct Ping
		{
			std::uint32_t unused : 10; // 10 bits for unused
			std::uint32_t ping : 10;   // 10 bits for ping
			std::uint32_t rest : 12;   // 12 bits for additional data

			Ping() : unused(0), ping(0), rest(0) {} // Constructor to initialize fields
		};
#pragma pack(pop)

#pragma pack(push, 1)
		struct ItemRefund
		{
			Main::Structures::ItemSerialInfo serialInfo{};
			std::uint32_t mpToAdd{};
		};
#pragma pack(pop)

#pragma pack(push, 1)
		struct ItemUpgrade
		{
			Main::Structures::ItemSerialInfo serialInfo{};
			std::uint32_t usedEnergy{};
		};
#pragma pack(pop)

#pragma pack(push, 1)
		struct MailboxMessage
		{
			char nickname[16]{};
			char message[256]{};
		};
#pragma pack(pop)

#pragma pack(push, 1)
		struct RoomInfo
		{
			std::uint16_t roomNumber{};
			std::uint16_t unknown{ 2 }; // seemingly always 2 for some reason
			char password[8]{};
		};
#pragma pack(pop)
	}
}
#endif