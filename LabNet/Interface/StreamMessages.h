#pragma once

#include <chrono>
#include <so_5/all.hpp>
#include "Interfaces.h"

namespace StreamMessages
{
	struct new_data_from_port
	{
		Interface::Interfaces interface;
		const char pin;
		std::shared_ptr<std::vector<char>> data;
		std::chrono::time_point<std::chrono::high_resolution_clock> time;
	};

	struct send_data_to_port
	{
		Interface::Interfaces interface;
		const char pin;
		std::shared_ptr<std::vector<char>> data;
	};

	struct send_data_complete
	{
		Interface::Interfaces interface;
		const char pin;
	};		 
}