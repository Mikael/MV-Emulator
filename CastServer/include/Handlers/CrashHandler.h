namespace Cast
{
    namespace Handlers
    {
        inline void handleCrash(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            try
            {
                auto& logger = ::Utils::Logger::getInstance(true);

                logger.log<::Utils::LogDestination::File>("User crashed (SessionID: " + std::to_string(session.getId()) + ", RoomNum: "
                    + std::to_string(session.getRoomNumber()) + ")", ::Utils::LogType::Info, "handleCrash");

                const auto selfSessionId = session.getId();
                const auto roomHostId = request.getSession();

                Main::Structures::UniqueId uniqueId{ selfSessionId, 4, 0 };
                auto response = request;
                response.setData(reinterpret_cast<std::uint8_t*>(&uniqueId), sizeof(uniqueId));

                // Forward the crash event to the room host
                roomsManager.playerForwardToHost(roomHostId, selfSessionId, response);

                // Remove player from the room and update the RoomsManager
                roomsManager.removePlayerFromRoom(session.getId());
                logger.log<::Utils::LogDestination::File>("Player removed from room after crash (SessionID: "
                    + std::to_string(session.getId()) + ")", ::Utils::LogType::Info, "handleCrash");
            }
            catch (const std::exception& e)
            {
                auto& logger = ::Utils::Logger::getInstance(true);
                logger.log<::Utils::LogDestination::File>(std::string("Exception in handleCrash: ") + e.what(), ::Utils::LogType::Error, "handleCrash");
            }
        }
    }
}
