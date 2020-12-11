#pragma once

#include <so_5/all.hpp>
#include "connection.h"

namespace LabNet::network
{
	struct ClientConnected final : public so_5::message_t
	{
            ClientConnected(std::shared_ptr<Connection> c)
			: connection(c)
		{
		};

		std::shared_ptr<Connection> connection;
	};
	
	struct ClientDisconnected final : public so_5::signal_t
	{
	};
}