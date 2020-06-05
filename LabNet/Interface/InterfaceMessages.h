#pragma once

#include <so_5/all.hpp>
#include <string>
#include <chrono>
#include "Interfaces.h"

namespace Interface
{
	struct reset_interface {};
	struct pause_interface {};
	struct continue_interface
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> time;
	};
	
	struct interface_init_result
	{
		Interface::Interfaces interface;
		bool is_succeed;
	};
	
	struct interface_lost
	{
		Interface::Interfaces interface;
	};

	struct interface_reconnected
	{
		Interface::Interfaces interface;
	};
}