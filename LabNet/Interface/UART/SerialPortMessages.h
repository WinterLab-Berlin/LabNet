#pragma once

#include <string>
#include <vector>
#include <memory>

namespace uart::messages
{
	struct init_port
	{
		const int port_id;
		const int baud;
	};

	struct init_port_result
	{
		const int port_id;
		bool is_succeed;
	};
	
	struct port_unexpected_closed
	{
		const int port_id;
		const int baud;
	};

	struct port_reconnected
	{
		const int port_id;
	};

	struct new_data_from_port
	{
		const int port_id;
		std::shared_ptr<std::vector<char>> data;
	};

	struct send_data_to_port
	{
		const int port_id;
		std::shared_ptr<std::vector<char>> data;
	};

	struct send_data_complete
	{
		const int port_id;
	};		 
}
