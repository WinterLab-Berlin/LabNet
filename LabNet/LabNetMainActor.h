#pragma once

#include <so_5/all.hpp>
#include <LoggingFacility.h>
#include "Network/Connection.h"

namespace LabNet
{
	class LabNetMainActor final : public so_5::agent_t
	{
	public:
		LabNetMainActor(context_t ctx, Logger logger, so_5::mbox_t digOutBox);
		~LabNetMainActor();

	private:
		void so_define_agent() override;
	
		so_5::state_t wait_for_connection {this, "waiting for connection"};
		so_5::state_t connected {this, "connected"};
	
		Logger _logger;
		std::shared_ptr<Connection> _connection;
		const so_5::mbox_t _gpioBox;
		const so_5::mbox_t _gpioWiringBox;
		const so_5::mbox_t _rfidBox;
		const so_5::mbox_t _uartBox;
		const so_5::mbox_t _digOutBox;
		const so_5::mbox_t _interfaceManager;
	};
}