#include "GPIOManager.h"
#include <wiringPi.h>
#include <chrono>
#include "../InitMessages.h"
#include "../InterfaceMessages.h"
#include "../DigitalMessages.h"
#include "Messages.h"

using namespace io_board;

GPIOManager::GPIOManager(context_t ctx, const so_5::mbox_t selfBox, const so_5::mbox_t parentBox, Logger logger)
	: so_5::agent_t(ctx)
	, _selfBox(selfBox)
	, _parentMbox(parentBox)
	, _logger(logger)
	, _interfaces(ctx.env().create_mbox("ManageInterfaces"))
{
	
	_outPins[1] = 5;
	_outPins[2] = 26;
	_outPins[3] = 6;
	_outPins[4] = 22;
	_outPins[5] = 21;
	_outPins[6] = 7;
	_outPins[7] = 1;
	_outPins[8] = 0;
	_outPins[9] = 4;
	_outPins[10] = 3;
	
	_inPins[1] = 23;
	_inPins[2] = 27;
	_inPins[3] = 28;
	_inPins[4] = 24;
	_inPins[5] = 25;
	_inPins[6] = 29;
	
	for (auto& out : _outPins)
	{
		_outputs[out.first] = DigitalOutput{ out.second, out.first };
	}
	
	for (auto& inp : _inPins)
	{
		_inputs[inp.first] = DigitalInput{ inp.second, inp.first };
	}
}

GPIOManager::~GPIOManager()
{
	_inputStateReader.reset();
}

void GPIOManager::so_define_agent()
{
	this >>= wait_for_init;
	
	wait_for_init
		.event(_selfBox,
		[this](mhood_t<init_interface> msg) {
				so_5::send<Interface::InitMessages::init_gpio_request>(_interfaces, _selfBox);
			})
		.event(_selfBox,
			[this](mhood_t<Interface::InitMessages::can_init_no> msg) {
				for (auto& out : _outputs)
					out.second.available = false;
	
				for (auto& in : _inputs)
					in.second.available = false;
				
				so_5::send<Interface::interface_init_result>(_parentMbox, Interface::GPIO_TOP_PLANE, false);
			})
		.event(_selfBox,
			[this](mhood_t<Interface::InitMessages::can_init_yes> msg) {
				so_5::send<Interface::interface_init_result>(_parentMbox, Interface::GPIO_TOP_PLANE, true);
				
				_inputStateReader = std::make_unique<DigitalInputStateReader>(_parentMbox, &_inputs, _logger);
				
				this >>= running;
			});
	
	running
		.event(_selfBox,
		[this](mhood_t<init_interface> msg) {
			so_5::send<Interface::interface_init_result>(_parentMbox, Interface::GPIO_TOP_PLANE, true);
		})
		.event(_selfBox,
		[this](mhood_t<init_digital_in> msg) {
				if (msg->pin > 0 && msg->pin < 7)
				{
					_inputs[msg->pin].resistor_state = msg->resistor_state;
					_inputs[msg->pin].is_inverted = msg->is_inverted;
					
					pinMode(_inputs[msg->pin].pin_h, INPUT);
					switch (_inputs[msg->pin].resistor_state)
					{
					case resistor::off:
						pullUpDnControl(_inputs[msg->pin].pin_h, PUD_OFF);
						break;
					case resistor::pull_down:
						pullUpDnControl(_inputs[msg->pin].pin_h, PUD_DOWN);
						break;
					case resistor::pull_up:
						pullUpDnControl(_inputs[msg->pin].pin_h, PUD_UP);
						break;
					}
					
					_inputs[msg->pin].available = true;
					_inputs[msg->pin].state = 2;
					
					so_5::send<DigitalMessages::digital_in_init_result>(_parentMbox, Interface::GPIO_TOP_PLANE, msg->pin, true);
				}
				else
				{
					so_5::send<DigitalMessages::digital_in_init_result>(_parentMbox, Interface::GPIO_TOP_PLANE, msg->pin, false);
				}
			})
		.event(_selfBox,
		[this](mhood_t<init_digital_out> msg) {
				if (msg->pin > 0 && msg->pin < 11)
				{
					_outputs[msg->pin].is_inverted = msg->is_inverted;
					_outputs[msg->pin].available = true;
					
					pinMode(_outputs[msg->pin].pin_h, OUTPUT);
					if (_outputs[msg->pin].is_inverted)
					{
						digitalWrite(_outputs[msg->pin].pin_h, 1);
					}
					else
					{
						digitalWrite(_outputs[msg->pin].pin_h, 0);
					}
					
					so_5::send<DigitalMessages::digital_out_init_result>(_parentMbox, Interface::GPIO_TOP_PLANE, msg->pin, true);
				}
				else
				{
					so_5::send<DigitalMessages::digital_out_init_result>(_parentMbox, Interface::GPIO_TOP_PLANE, msg->pin, false);
				}
			})
		.event(_selfBox,
		[this](mhood_t<DigitalMessages::set_digital_out> msg) {
				if (msg->pin > 0 && msg->pin < 11)
				{
					if (_outputs[msg->pin].is_inverted)
					{
						digitalWrite(_outputs[msg->pin].pin_h, !msg->state);
					}
					else
					{
						digitalWrite(_outputs[msg->pin].pin_h, msg->state);
					}
		
					//_logger->writeInfoEntry(string_format("gpio set %d", msg->state));
					so_5::send<DigitalMessages::return_digital_out_state>(msg->mbox, Interface::GPIO_TOP_PLANE, msg->pin, msg->state, std::chrono::high_resolution_clock::now());
				}
				else
				{
					so_5::send<DigitalMessages::invalid_digital_out_pin>(msg->mbox, Interface::GPIO_TOP_PLANE, msg->pin);
				}
			})
		.event(_selfBox,
			[this](mhood_t<Interface::pause_interface> msg) {
				_inputStateReader.reset();
				
				this >>= paused;
			})
		.event(_selfBox,
			[this](mhood_t<Interface::reset_interface> msg) {
				this >>= wait_for_init;
				
				for(auto& in : _inputs)
				{
					in.second.available = false;
				}
				
				for (auto& out : _outputs)
				{
					out.second.available = false;
				}
			});
	
	paused
		.event(_selfBox,
			[this](mhood_t<Interface::reset_interface> msg) {
				this >>= wait_for_init;
				
				for (auto& in : _inputs)
				{
					in.second.available = false;
				}
				
				for (auto& out : _outputs)
				{
					out.second.available = false;
				}
		})
		.event(_selfBox,
			[this](mhood_t<Interface::continue_interface>) {
				_inputStateReader = std::make_unique<DigitalInputStateReader>(_parentMbox, &_inputs, _logger);
				
				this >>= running;
			});
			
}

