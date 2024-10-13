#ifndef ACCOUNT_INFO_HANDLER_MAIN_H
#define ACCOUNT_INFO_HANDLER_MAIN_H

#include "Network/Session.h"
#include "Network/Packet.h"
#include "../../Structures/AccountInfo/MainAccountInfo.h"
#include "../../MainEnums.h"
#include "Utils/Utils.h"
#include "../../../include/Network/MainSession.h"
#include "../../../include/Network/MainSessionManager.h"

namespace Main
{
    namespace Handlers
    {
        inline void handleAccountInformation(const Common::Network::Packet& request, Main::Network::Session& session,
            Main::Network::SessionsManager& sessionsManager, Main::Persistence::PersistentDatabase& database,
            Main::Structures::AccountInfo& accountInfo, std::uint64_t timeSinceLastServerRestart, std::uint32_t extra = 1)
        {
            Common::Network::Packet response;
            response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
            response.setOrder(413);
            response.setExtra(extra);

            // AccountInfo closure packet
            if (extra == 59)
            {
                static std::array<std::uint8_t, 28> unused = {}; // Use a static array initialized to zero
                response.setData(unused.data(), unused.size());
                session.asyncWrite(response);
                return;
            }

            accountInfo.uniqueId.session = session.getId();
            accountInfo.uniqueId.server = 4; // currently hardcoded
            accountInfo.serverTime = accountInfo.getUtcTimeMs() - timeSinceLastServerRestart;

            response.setData(reinterpret_cast<std::uint8_t*>(&accountInfo), sizeof(accountInfo));
            session.asyncWrite(response);
            session.setAccountInfo(accountInfo);
            sessionsManager.addSession(&session);

            // Load the user's friend list and blocked players from the database
            auto friends = database.loadFriends(accountInfo.accountID);
            session.setFriendList(friends);

            // Find all the online friends and set them
            for (const auto& currentFriend : friends)
            {
                if (auto targetSession = sessionsManager.getSessionByAccountId(currentFriend.targetAccountId))
                {
                    // Notify online friends
                    session.updateFriendSession(targetSession);
                    targetSession->updateFriendSession(&session);
                    targetSession->logFriend(46, accountInfo.accountID);
                }
            }

            session.setBlockedPlayers(database.loadBlockedPlayers(accountInfo.accountID));
            session.setMute(database.isMuted(accountInfo.accountID));

            // Load mailboxes only once
            auto mailboxReceived = database.loadMailboxes(accountInfo.accountID, true);
            auto mailboxSent = database.loadMailboxes(accountInfo.accountID, false);
            session.setMailbox(mailboxReceived, true);
            session.setMailbox(mailboxSent, false);

            // Check for new mailboxes
            auto newMailboxes = database.getNewMailboxes(accountInfo.accountID);
            if (!newMailboxes.empty())
            {
                // Send online messages (mailbox) if there are new ones
                response.setOrder(104);
                response.setOption(newMailboxes.size());
                response.setExtra(Main::Enums::MailboxExtra::MAILBOX_RECEIVED);
                response.setData(reinterpret_cast<std::uint8_t*>(newMailboxes.data()), newMailboxes.size() * sizeof(Main::Structures::Mailbox));
                session.asyncWrite(response);
            }
        }
    }
}

#endif
