#ifndef AUTH_CHANNELS_HANDLER_H
#define AUTH_CHANNELS_HANDLER_H


#include <array>

#include "Network/Session.h"
#include "Network/Packet.h"
#include "../Structures/AuthChannels.h"
#include "../AuthEnums.h"


#include <array>
#include <algorithm> // For std::copy

#include <array>

namespace Auth
{
	namespace Handlers
	{
		inline void handleServerChannelsInfo(const Common::Network::Packet& request, Common::Network::Session& session)
		{
			Auth::Structures::ChannelsInfo channelsInfo{};
			using Status = Auth::Enums::ChannelStatus;

			// Here, we mimic your original setup with specific statuses.
			std::array<std::uint32_t, 6> channels{
				   Status::OFFLINE,
				   Status::OFFLINE,
				   Status::OFFLINE,
				   Status::LOW_TRAFFIC,
				   Status::OFFLINE,
				   Status::OFFLINE
			};

			// If you have a function to get the actual channel status, you could loop through and set them dynamically.
			// Example (replace this with your actual function):
			for (size_t i = 0; i < channels.size(); ++i)
			{
				channels[i] = /* getChannelStatus(i) */ Status::LOW_TRAFFIC; // Placeholder: replace with actual logic
			}

			// Or keep your original static statuses if that's intended.
			// std::array<std::uint32_t, 6> channels {
			//     Status::OFFLINE, Status::OFFLINE, Status::OFFLINE, Status::LOW_TRAFFIC, Status::OFFLINE, Status::OFFLINE
			// };

			channelsInfo.initializeChannels(channels);

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setCommand(23, 0, 0, static_cast<std::uint8_t>(channels.size()));
			response.setData(reinterpret_cast<std::uint8_t*>(&channelsInfo), sizeof(channelsInfo));
			session.asyncWrite(response);
		}
	}
}

#endif