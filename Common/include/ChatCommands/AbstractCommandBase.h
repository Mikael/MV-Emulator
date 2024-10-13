#ifndef COMMON_ACOMMANDBASE_HEADER
#define COMMON_ACOMMANDBASE_HEADER

#include "../Enums/PlayerEnums.h"
#include <string>
#include "../Network/Session.h"
#include "../Network/Packet.h"

namespace Common
{
	namespace Command
	{
		template<typename SessionType, typename SessionManagerType>
		class AbstractCommandBase
		{
		protected:
			std::string m_confirmationMessage;
			Common::Enums::PlayerGrade m_requiredGrade;

		public:
			explicit AbstractCommandBase(Common::Enums::PlayerGrade playerGrade)
				: m_confirmationMessage{ std::string(16, '0') }
				, m_requiredGrade{ playerGrade }
			{}

			Common::Enums::PlayerGrade getRequiredGrade() const
			{
				return m_requiredGrade;
			}

			void sendConfirmation(Common::Network::Packet& response, SessionType& session)
			{
				response.setOrder(316);
				response.setExtra(1);

				if (m_confirmationMessage.size() > std::numeric_limits<std::uint16_t>::max()) {
					throw std::runtime_error("Confirmation message size exceeds uint16_t limit");
				}

				response.setData(reinterpret_cast<std::uint8_t*>(m_confirmationMessage.data()), m_confirmationMessage.size());
				response.setOption(static_cast<std::uint16_t>(m_confirmationMessage.size())); // Cast safely to uint16_t
				response.setMission(0);

				session.asyncWrite(response);
				m_confirmationMessage.resize(16);
			}


			virtual ~AbstractCommandBase() = default;
			virtual void getCommandUsage(SessionType& session, Common::Network::Packet& response) = 0;
		};
	}
}
#endif