#pragma once

#include <so_5/all.hpp>

namespace GPIO
{
	enum resistor
	{
		off,
		pull_down,
		pull_up
	};
	
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
	
	struct pin_init_success
	{
		const char pin;
	};
	
	struct pin_init_failed
	{
		const char pin;
	};
	
	struct set_digital_out
	{
		const char pin;
		const bool state;
		const so_5::mbox_t mbox;
	};
	
	struct return_digital_out_state
	{
		const char pin;
		const bool state;
	};
	
	struct return_digital_in_state
	{
		const char pin;
		const bool state;
	};
}