#pragma once

#include <so_5/all.hpp>
#include <map>
#include <vector>
#include <LoggingFacility.h>
#include "Network/Connection.h"
#include <google/protobuf/message.h>

namespace LabNet
{
	class LabNetMainActor final : public so_5::agent_t
	{
	public:
		LabNetMainActor(context_t ctx, Logger logger);
		~LabNetMainActor();

	private:
		void so_define_agent() override;

		template<class T>
		void send_message(T& mes, const std::string& mes_name)
		{
			auto it = _receiver.find(mes_name);
			if (it != _receiver.end())
			{
				for (auto& rec : it->second)
				{
					so_5::send<T>(rec, mes);
				}
			}
		};

		template<class T>
		void send_message(T& mes)
		{
			std::string mes_name = mes->GetTypeName();
			send_message(mes, mes_name);
		};

		
	
		so_5::state_t wait_for_connection_state {this, "wait for connection_state"};
		so_5::state_t connected_state {this, "connected_state"};
		so_5::state_t reset_state { this, "reset_state" };
		so_5::state_t reset_no_connection_state{ this, "reset_no_connection_state" };
	
		Logger _logger;
		std::shared_ptr<Connection> _connection;
		std::map<std::string, std::vector<so_5::mbox_t>> _receiver;
	};
}