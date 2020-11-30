#pragma once

#include <so_5/all.hpp>
#include "Connection.h"

namespace LabNet::network
{
	struct client_connected final : public so_5::message_t
	{
            client_connected(std::shared_ptr<Connection> c)
			: connection(c)
		{
		};

		std::shared_ptr<Connection> connection;
	};
	
	struct client_disconnected final : public so_5::signal_t
	{
	};

	struct RegisterForMessage final : public so_5::message_t
	{
		RegisterForMessage(std::string message, so_5::mbox_t rec)
			: message_name(message)
			, receiver(rec)
		{
		};

		std::string message_name;
		so_5::mbox_t receiver;
	};
}