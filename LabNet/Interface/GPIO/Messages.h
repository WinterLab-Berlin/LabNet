#pragma once

#include <so_5/all.hpp>
#include "DigitalInput.h"

namespace GPIO
{
	struct init_interface final : public so_5::signal_t {};
	struct init_failed final : public so_5::signal_t {};
	struct init_succeed final : public so_5::signal_t {};
	
	struct init_digital_in
	{
		const char pin;
		const resistor resistor_state = off;
		const bool is_inverted = false;
	};
	
	struct digital_in_init_success
	{
		const char pin;
	};
	
	struct digital_in_init_failed
	{
		const char pin;
	};
	
	struct init_digital_out
	{
		const char pin;
		const bool is_inverted = false;
	};
	
	struct digital_out_init_success
	{
		const char pin;
	};
	
	struct digital_out_init_failed
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