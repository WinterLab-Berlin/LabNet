#include "GPIOManager.h"
#include <wiringPi.h>
#include "../InitMessages.h"
#include "../InterfaceMessages.h"
#include "Messages.h"

GPIO::GPIOManager::GPIOManager(context_t ctx, const so_5::mbox_t mbox, Logger logger)
	: so_5::agent_t(ctx)
	, _parentMbox(mbox)
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
		_outputs[out.first] = DigitalOutput{ out.first, out.second };
	}
	
	for (auto& inp : _inPins)
	{
		_inputs[inp.first] = DigitalInput{ inp.first, inp.second };
	}
}

GPIO::GPIOManager::~GPIOManager()
{
	_inputStateReader.reset();
}

void GPIO::GPIOManager::so_define_agent()
{
	this >>= wait_for_init;
	
	wait_for_init
		.event(so_direct_mbox(),
			[this](mhood_t<init_interface> msg) {
				so_5::send<Interface::InitMessages::init_gpio_request>(_interfaces, so_direct_mbox());
			})
		.event(so_direct_mbox(),
			[this](mhood_t<Interface::InitMessages::can_init_no> msg) {
				for (auto& out : _outputs)
					out.second.available = false;
	
				for (auto& in : _inputs)
					in.second.available = false;
				
				so_5::send <init_failed>(_parentMbox);
			})
		.event(so_direct_mbox(),
			[this](mhood_t<Interface::InitMessages::can_init_yes> msg) {
				so_5::send <init_succeed>(_parentMbox);
				
				_inputStateReader = std::make_unique<DigitalInputStateReader>(_parentMbox, _inputs);
				
				this >>= running;
			});
	
	running
		.event(so_direct_mbox(),
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
					
					so_5::send <digital_in_init_success>(_parentMbox, msg->pin);
				}
				else
				{
					so_5::send <digital_in_init_failed>(_parentMbox, msg->pin);
				}
			})
		.event(so_direct_mbox(),
			[this](mhood_t<init_digital_out> msg) {
				if (msg->pin > 0 && msg->pin < 11)
				{
					_outputs[msg->pin].is_inverted = msg->is_inverted;
					_outputs[msg->pin].available = true;
					
					pinMode(_outputs[msg->pin].pin_h, OUTPUT);
					
					so_5::send <digital_out_init_success>(_parentMbox, msg->pin);
				}
				else
				{
					so_5::send <digital_out_init_failed>(_parentMbox, msg->pin);
				}
			})
		.event(so_direct_mbox(),
			[this](mhood_t<set_digital_out> msg) {
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
		
					so_5::send <return_digital_out_state>(msg->mbox, msg->pin, msg->state);
				}
			})
		.event(so_direct_mbox(),
			[this](mhood_t<Interface::pause_all_interface> msg) {
				_inputStateReader.reset();
				
				this >>= paused;
			})
		.event(so_direct_mbox(),
			[this](mhood_t<Interface::reset_all_interface> msg) {
				this >>= wait_for_init;
				
				for(auto in : _inputs)
				{
					in.second.available = false;
				}
				
				for (auto out : _outputs)
				{
					out.second.available = false;
				}
			});
	
	paused
		.event(so_direct_mbox(),
			[this](mhood_t<Interface::reset_all_interface> msg) {
				this >>= wait_for_init;
				
				for (auto in : _inputs)
				{
					in.second.available = false;
				}
				
				for (auto out : _outputs)
				{
					out.second.available = false;
				}
		})
		.event(so_direct_mbox(),
			[this](mhood_t<Interface::continue_all_interface>) {
				_inputStateReader = std::make_unique<DigitalInputStateReader>(_parentMbox, _inputs);
				
				this >>= running;
			});
			
}

