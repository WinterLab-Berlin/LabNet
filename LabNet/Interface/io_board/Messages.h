#pragma once

#include <so_5/all.hpp>
#include "DigitalInput.h"

namespace io_board
{
	struct init_interface final : public so_5::signal_t {};
	
	struct init_digital_in
	{
		const char pin;
		const resistor resistor_state = off;
		const bool is_inverted = false;
	};
	
	struct init_digital_out
	{
		const char pin;
		const bool is_inverted = false;
	};
}