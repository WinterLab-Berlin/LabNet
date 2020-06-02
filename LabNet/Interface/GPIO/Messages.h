#pragma once

#include <so_5/all.hpp>
#include "DigitalInput.h"

namespace GPIO
{
	struct init_interface final : public so_5::signal_t {};
	struct interface_init_result
	{
		bool is_succeed;
	};
	
	struct init_digital_in
	{
		const char pin;
		const resistor resistor_state = off;
		const bool is_inverted = false;
	};
	
	struct digital_in_init_result
	{
		const char pin;
		bool is_succeed;
	};
	
	struct return_digital_in_state
	{
		const char pin;
		const bool state;
	};
	
	struct init_digital_out
	{
		const char pin;
		const bool is_inverted = false;
	};
	
	struct digital_out_init_result
	{
		const char pin;
		bool is_succeed;
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
	
	struct invalid_digital_out_pin
	{
		const char pin;
	};
}