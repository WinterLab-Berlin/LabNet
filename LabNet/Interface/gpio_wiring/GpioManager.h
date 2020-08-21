#pragma once

#include <map>
#include <vector>
#include <so_5/all.hpp>
#include <LoggingFacility.h>
#include "DigitalInput.h"
#include "DigitalOutput.h"
#include "DigitalInputStateReader.h"

namespace gpio_wiring
{
	class GpioManager final : public so_5::agent_t
	{
	public:
		GpioManager(context_t ctx, const so_5::mbox_t selfBox, const so_5::mbox_t parentBox, Logger logger);
		~GpioManager();
		
	private:
		void so_define_agent() override;
		
		so_5::state_t wait_for_init {this, "waiting for init"};
		so_5::state_t running {this, "running"};
		so_5::state_t paused {this, "paused"};
		
		std::map<char, char> _pins;
		std::map<int, DigitalOutput> _outputs;
		std::unique_ptr<DigitalInputStateReader> _inputStateReader;
		
		Logger _logger;
		const so_5::mbox_t _parentMbox;
		const so_5::mbox_t _interfaces;
		const so_5::mbox_t _selfBox;
		so_5::mchain_t _reader_box;
	};
}