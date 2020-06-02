#pragma once

#include <so_5/all.hpp>
#include <LoggingFacility.h>
#include "PinId.h"
#include <vector>
#include <chrono>

namespace DigitalOut
{
	struct abort_loop_helper
	{
	};
	
	struct pause_loop_helper
	{
	};
	
	struct continue_loop_helper
	{
	};
	
	struct DigitalOutputParameter
	{
		PinId id;
		uint offset;
		uint duration;
		bool handled = false;
		bool state = false;
	};
	
	class LoopHelper final : public so_5::agent_t
	{
	public:
		LoopHelper(context_t ctx, Logger logger, so_5::mbox_t dig_out_box, so_5::mbox_t lab_net_box);
		~LoopHelper();
		
	private:
		void so_define_agent() override;
		void turn_pin_on(PinId& id);
		void turn_pin_off(PinId& id);
		
		so_5::state_t _waitState {this, "wait"};
		so_5::state_t _runningState {this, "running"};
		so_5::state_t _pauseState {this, "pause"};
		
		std::string _loopName;
		Logger _logger;
		const so_5::mbox_t _labNetBox;
		const so_5::mbox_t _digOutBox;
		so_5::mbox_t _gpioBox;
		so_5::mbox_t _uartBox;
		
		unsigned int _loopPause;
		std::vector<DigitalOutputParameter> _loop;
		std::chrono::time_point<std::chrono::high_resolution_clock> _start;
	};
}