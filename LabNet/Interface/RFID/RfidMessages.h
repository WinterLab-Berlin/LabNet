#pragma once

#include <so_5/mbox.hpp>

namespace RFID
{
	struct init_interface
	{
		uint32_t antenna_phase1 = 0xFFFFFFFF;
		uint32_t antenna_phase2 = 0xFFFFFFFF;
		uint32_t phase_duration = 250;
		bool inverted = false;
	};
	
	struct set_phase_matrix
	{
		uint32_t antenna_phase1;
		uint32_t antenna_phase2;
		uint32_t phase_duration;
	};
	
	struct set_signal_inversion
	{
		bool inverted;
	};
	
	struct new_data
	{
		const int port_id;
		std::shared_ptr<std::vector<char>> data;
	};
}