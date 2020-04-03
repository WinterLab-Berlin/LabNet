#pragma once

#include <map>
#include <so_5/all.hpp>
#include <LoggingFacility.h>
#include "DigitalInput.h"
#include "DigitalOutput.h"
#include "DigitalInputStateReader.h"

namespace GPIO
{
	class GPIOManager final : public so_5::agent_t
	{
	public:
		GPIOManager(context_t ctx, const so_5::mbox_t mbox, Logger logger);
		~GPIOManager();
		
	private:
		void so_define_agent() override;
		
		so_5::state_t wait_for_init {this, "waiting for init"};
		so_5::state_t running {this, "running"};
		so_5::state_t paused {this, "paused"};
		
		std::map<char, char> _outPins;
		std::map<char, char> _inPins;
		std::map<int, DigitalInput> _inputs;
		std::map<int, DigitalOutput> _outputs;
		std::unique_ptr<DigitalInputStateReader> _inputStateReader;
		
		Logger _logger;
		const so_5::mbox_t _parentMbox;
		const so_5::mbox_t _interfaces;
	};
}