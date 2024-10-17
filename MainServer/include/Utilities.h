#ifndef DETAILS_HEADER_H
#define DETAILS_HEADER_H

#include <ConstantDatabase/Structures/SetItemInfo.h>
#include <Utils/Utils.h>
#include "Network/MainSession.h"
#include "Classes/RoomsManager.h"
#include "Network/Packet.h"

#include <chrono>

namespace Main
{
	namespace Details
	{
		namespace Constants
		{
			constexpr inline std::uint32_t maxEquippedItems = 17;
		}

		template<typename T>
		T parseData(const Common::Network::Packet& request, std::uint32_t offset = 0)
		{
			// this is not always necessarily true:
			// assert(sizeof(T) == request.getDataSize());

			T t;
			std::memcpy(&t, request.getData() + offset, request.getDataSize());
			return t;
		}
	}
}

#endif