#include "GPIOManager.h"
#include <wiringPi.h>

GPIO::GPIOManager::GPIOManager(context_t ctx, const so_5::mbox_t mbox, Logger logger)
	: so_5::agent_t(ctx)
	, _parentMbox(mbox)
	, _logger(logger)
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
}

GPIO::GPIOManager::~GPIOManager()
{
}

void GPIO::GPIOManager::so_define_agent()
{
	so_subscribe_self()
		.event(&GPIO::GPIOManager::init_digital_in_event)
		.event(&GPIO::GPIOManager::init_digital_out_event)
		.event(&GPIO::GPIOManager::check_inputs_event)
		.event(&GPIO::GPIOManager::set_digital_out_event);
}

void GPIO::GPIOManager::so_evt_start()
{
	so_5::send_delayed<check_inputs>(so_direct_mbox(), std::chrono::milliseconds(1));
}

void GPIO::GPIOManager::init_digital_in_event(const init_digital_in& ev)
{
	if (ev.pin > 0 && ev.pin < 7)
	{
		pinMode(_inPins[ev.pin], INPUT);
		switch (ev.resistor_state)
		{
		case resistor::off:
			pullUpDnControl(_inPins[ev.pin], PUD_OFF);
			break;
		case resistor::pull_down:
			pullUpDnControl(_inPins[ev.pin], PUD_DOWN);
			break;
		case resistor::pull_up:
			pullUpDnControl(_inPins[ev.pin], PUD_UP);
			break;
		}
		
		_inputs[ev.pin] = std::make_unique<DigitalInput>(_inPins[ev.pin], ev.pin, ev.is_inverted);
		so_5::send < pin_init_success>(_parentMbox, ev.pin);
	}
	else
	{
		so_5::send < pin_init_failed>(_parentMbox, ev.pin);
	}
}

void GPIO::GPIOManager::init_digital_out_event(const init_digital_out& ev)
{
	if (ev.pin > 0 && ev.pin < 11)
	{
		pinMode(_outPins[ev.pin], OUTPUT);
		_outputs[ev.pin] = std::make_unique<DigitalOutput>(_outPins[ev.pin], ev.pin, ev.is_inverted);
		so_5::send < pin_init_success>(_parentMbox, ev.pin);
		
		if (ev.is_inverted)
		{
			digitalWrite(_outputs[ev.pin]->pin_h, 1);
		}
		else
		{
			digitalWrite(_outputs[ev.pin]->pin_h, 0);
		}
		
		so_5::send <return_digital_out_state>(_parentMbox, ev.pin, false);
	}
	else
	{
		so_5::send < pin_init_failed>(_parentMbox, ev.pin);
	}
}

void GPIO::GPIOManager::check_inputs_event(const check_inputs& ev)
{
	int res = 0;
	for (auto it = _inputs.begin(); it != _inputs.end(); it++)
	{
		res = digitalRead(it->second->pin_h);
		if (res != it->second->state)
		{
			it->second->state = res;
			if (it->second->is_inverted)
				so_5::send<return_digital_in_state>(_parentMbox, it->second->pin_l, !res);
			else
				so_5::send<return_digital_in_state>(_parentMbox, it->second->pin_l, res);
		}
	}
	
	so_5::send_delayed<check_inputs>(so_direct_mbox(), std::chrono::milliseconds(1));
}

void GPIO::GPIOManager::set_digital_out_event(const set_digital_out& ev)
{
	auto it = _outputs.find(ev.pin);
	if (it != _outputs.end())
	{
		if (it->second->is_inverted)
		{
			digitalWrite(_outputs[ev.pin]->pin_h, !ev.state);
		}
		else
		{
			digitalWrite(_outputs[ev.pin]->pin_h, ev.state);
		}
		
		so_5::send <return_digital_out_state>(ev.mbox, ev.pin, ev.state);
	}
}