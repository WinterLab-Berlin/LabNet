#pragma once

#include <so_5/mbox.hpp>

namespace SAM
{
	struct init_interface final : public so_5::signal_t {};
	struct init_succeed final : public so_5::signal_t {};
	struct init_failed final : public so_5::signal_t {};
	
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
}