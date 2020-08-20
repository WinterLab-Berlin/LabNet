#pragma once

namespace uart::private_messages
{
	struct try_to_reconnect
	{
		const int port_id;
		const int baud;
	};
	
	struct send_data_complete
	{
		const char pin;
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
}