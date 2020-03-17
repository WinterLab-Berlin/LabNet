#pragma once

#include <map>
#include <so_5/all.hpp>
#include <LoggingFacility.h>
#include "Messages.h"
#include "DigitalInput.h"
#include "DigitalOutput.h"

namespace GPIO
{
	struct check_inputs final : public so_5::message_t
	{
	};
	
	class GPIOManager final : public so_5::agent_t
	{
	public:
		GPIOManager(context_t ctx, const so_5::mbox_t mbox, Logger logger);
		~GPIOManager();

		void so_define_agent() override;
		void so_evt_start() override;
		
	private:
		void init_digital_in_event(const init_digital_in& ev);
		void init_digital_out_event(const init_digital_out& ev);
		void check_inputs_event(const check_inputs& ev);
		void set_digital_out_event(const set_digital_out& ev);
		
		
		std::map<char, char> _outPins;
		std::map<char, char> _inPins;
		std::map<int, std::unique_ptr<DigitalInput>> _inputs;
		std::map<int, std::unique_ptr<DigitalOutput>> _outputs;
		const so_5::mbox_t _parentMbox;
		Logger _logger;
	};
}