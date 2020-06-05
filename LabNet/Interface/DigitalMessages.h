#pragma once

#include <chrono>
#include <so_5/all.hpp>
#include "Interfaces.h"

namespace DigitalMessages
{
	struct digital_in_init_result
	{
		Interface::Interfaces interface;
		const char pin;
		bool is_succeed;
	};
	
	struct return_digital_in_state
	{
		Interface::Interfaces interface;
		const char pin;
		const bool state;
		std::chrono::time_point<std::chrono::high_resolution_clock> time;
	};
	
	struct digital_out_init_result
	{
		Interface::Interfaces interface;
		const char pin;
		bool is_succeed;
	};
	
	struct set_digital_out
	{
		Interface::Interfaces interface;
		const char pin;
		const bool state;
		const so_5::mbox_t mbox;
	};
	
	struct return_digital_out_state
	{
		Interface::Interfaces interface;
		const char pin;
		const bool state;
		std::chrono::time_point<std::chrono::high_resolution_clock> time;
	};
	
	struct invalid_digital_out_pin
	{
		Interface::Interfaces interface;
		const char pin;
	};
}