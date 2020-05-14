#pragma once

namespace uart::private_messages
{
	struct try_to_reconnect
	{
		const int port_id;
		const int baud;
	};
}