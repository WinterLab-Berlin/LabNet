#pragma once

#include <map>
#include <vector>
#include <so_5/all.hpp>
#include <LoggingFacility.h>
#include "PinId.h"

namespace DigitalOut
{
	class DigitalOutHelper final : public so_5::agent_t
	{
	public:
		DigitalOutHelper(context_t ctx, Logger logger, so_5::mbox_t selfBox, so_5::mbox_t parentBox);
		~DigitalOutHelper();
		
		
	private:
		void so_define_agent() override;
		
		so_5::state_t _runningState {this, "running"};
		so_5::state_t _pausedState {this, "paused"};
		
		Logger _logger;
		const so_5::mbox_t _selfBox;
		const so_5::mbox_t _labNetBox;
		const so_5::mbox_t _gpioBox;
		const so_5::mbox_t _uartBox;
		
		std::map<std::string, so_5::mbox_t> _loopHelper;
		std::map<PinId, so_5::mbox_t> _pulseHelper;
	};
}