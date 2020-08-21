#include "GpioManager.h"
#include <wiringPi.h>
#include <chrono>
#include "../InitMessages.h"
#include "../InterfaceMessages.h"
#include "../DigitalMessages.h"
#include "Messages.h"

using namespace gpio_wiring;

enum pin_type
{
	None = 0,
	Out  = 1,
	In   = 2
};

GpioManager::GpioManager(context_t ctx, const so_5::mbox_t selfBox, const so_5::mbox_t parentBox, Logger logger)
	: so_5::agent_t(ctx)
	, _selfBox(selfBox)
	, _parentMbox(parentBox)
	, _logger(logger)
	, _interfaces(ctx.env().create_mbox("ManageInterfaces"))
{
	std::vector<char> pin = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 };
	for (int i = 0; i < pin.size(); i++)
	{
		_pins[i] = pin_type::None;
	}
}

GpioManager::~GpioManager()
{
	_inputStateReader.reset();
}

void GpioManager::so_define_agent()
{
	this >>= wait_for_init;
	
	wait_for_init
		.event(_selfBox,
		[this](mhood_t<init_interface> msg)
		{
			so_5::send<Interface::InitMessages::init_gpio_wiring_request>(_interfaces, _selfBox);
		})
		.event(_selfBox,
		[this](mhood_t<Interface::InitMessages::can_init_no> msg)
		{
			so_5::send<Interface::interface_init_result>(_parentMbox, Interface::GPIO_WIRING, false);
		})
		.event(_selfBox,
		[this](mhood_t<Interface::InitMessages::can_init_yes> msg)
		{
			so_5::send<Interface::interface_init_result>(_parentMbox, Interface::GPIO_WIRING, true);
			
			_reader_box = so_environment().create_mchain(so_5::make_unlimited_mchain_params());
			_inputStateReader = std::make_unique<DigitalInputStateReader>(_parentMbox, _reader_box, _logger);
				
			this >>= running;
		});
	
	running
		.event(_selfBox,
		[this](mhood_t<init_interface> msg)
		{
			so_5::send<Interface::interface_init_result>(_parentMbox, Interface::GPIO_WIRING, true);
		})
		.event(_selfBox,
		[this](mhood_t<init_digital_in> msg)
		{
			std::map<char, char>::iterator it = _pins.find(msg->pin);
			if (it != _pins.end() && it->second != pin_type::Out)
			{
				if (it->second == pin_type::None)
				{
					it->second = pin_type::In;
					
					so_5::send<DigitalInput>(_reader_box, msg->pin, msg->is_inverted, 2, msg->resistor_state);
				}
				
				so_5::send<DigitalMessages::digital_in_init_result>(_parentMbox, Interface::GPIO_WIRING, msg->pin, true);
			}
			else
			{
				so_5::send<DigitalMessages::digital_in_init_result>(_parentMbox, Interface::GPIO_WIRING, msg->pin, false);
			}
		})
		.event(_selfBox,
		[this](mhood_t<init_digital_out> msg)
		{
			std::map<char, char>::iterator it = _pins.find(msg->pin);
			if (it != _pins.end() && it->second != pin_type::In)
			{
				if (it->second == pin_type::None)
				{
					it->second = pin_type::Out;
					_outputs[msg->pin] = DigitalOutput { msg->pin, msg->is_inverted };
					
					pinMode(msg->pin, OUTPUT);
					if (msg->is_inverted)
					{
						digitalWrite(msg->pin, 1);
					}
					else
					{
						digitalWrite(msg->pin, 0);
					}
				}
				
				so_5::send<DigitalMessages::digital_out_init_result>(_parentMbox, Interface::GPIO_WIRING, msg->pin, true);
			}
			else
			{
				so_5::send<DigitalMessages::digital_out_init_result>(_parentMbox, Interface::GPIO_WIRING, msg->pin, false);
			}
		})
		.event(_selfBox,
		[this](mhood_t<DigitalMessages::set_digital_out> msg)
		{
			std::map<int, DigitalOutput>::iterator it = _outputs.find(msg->pin);
			if (it != _outputs.end())
			{
				if (_outputs[msg->pin].is_inverted)
				{
					digitalWrite(_outputs[msg->pin].pin, !msg->state);
				}
				else
				{
					digitalWrite(_outputs[msg->pin].pin, msg->state);
				}
				
				so_5::send<DigitalMessages::return_digital_out_state>(msg->mbox, Interface::GPIO_WIRING, msg->pin, msg->state, std::chrono::high_resolution_clock::now());
			}
			else
			{
				so_5::send<DigitalMessages::invalid_digital_out_pin>(msg->mbox, Interface::GPIO_WIRING, msg->pin);
			}
		})
		.event(_selfBox,
		[this](mhood_t<Interface::pause_interface> msg)
		{
			so_5::send<Interface::pause_interface>(_reader_box);
				
			this >>= paused;
		})
		.event(_selfBox,
		[this](mhood_t<Interface::reset_interface> msg)
		{
			_outputs.clear();
			_inputStateReader.reset();
			
			for (auto& in : _pins)
			{
				in.second = pin_type::None;
			}
			
			this >>= wait_for_init;
		});
	
	paused
		.event(_selfBox,
		[this](mhood_t<Interface::reset_interface> msg)
		{
			_outputs.clear();
			_inputStateReader.reset();
			
			for (auto& in : _pins)
			{
				in.second = pin_type::None;
			}
			
			this >>= wait_for_init;
		})
		.event(_selfBox,
		[this](mhood_t<Interface::continue_interface> msg)
		{
			so_5::send<Interface::continue_interface>(_reader_box);
				
			this >>= running;
		});
			
}

