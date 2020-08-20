#pragma once

#include "../Interfaces.h"

namespace uart::messages
{
	struct init_port
	{
		Interface::Interfaces port_id;
		const int baud;
	};
}
