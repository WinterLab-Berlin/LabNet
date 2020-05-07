#pragma once

#include <so_5/all.hpp>
#include "Network/Connection.h"

namespace LabNet
{
	class Connected
	{
	public:
		Connected(std::shared_ptr<Connection> c)
			: connection(c)
		{
		};
		
		std::shared_ptr<Connection> connection;
	};
	
	struct Disconnected final : public so_5::signal_t
	{
	};
	
	
}