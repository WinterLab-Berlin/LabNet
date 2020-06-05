#pragma once

#include <so_5/all.hpp>
#include <string>
#include "Interfaces.h"

namespace Interface
{
	struct reset_interface {};
	struct pause_interface {};
	struct continue_interface {};
	
	struct interface_init_result
	{
		Interface::Interfaces interface;
		bool is_succeed;
	};
	
	
}