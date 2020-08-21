#include "DigitalInputStateReader.h"
#include "Messages.h"
#include <wiringPi.h>
#include <chrono>
#include "../DigitalMessages.h"
#include "../InterfaceMessages.h"

using namespace gpio_wiring;

DigitalInputStateReader::DigitalInputStateReader(const so_5::mbox_t parent, const so_5::mchain_t reader_box, Logger logger)
	: _parent(parent)
	, _inputs()
	, _reader_box(reader_box)
	, _future_obj(_exit_signal.get_future())
	, _logger(logger)
	, _check_inputs(true)
{
	std::thread read_worker{&DigitalInputStateReader::data_read_thread, this};
	_read_worker = std::move(read_worker);
}

DigitalInputStateReader::~DigitalInputStateReader()
{
	_exit_signal.set_value();
	_read_worker.join();
}

bool DigitalInputStateReader::stop_requested()
{
	// checks if value in future object is available
	if(_future_obj.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
	    return false;
	return true;
}

void DigitalInputStateReader::box_msg_handler()
{
	using namespace std::chrono_literals;
	
	receive(from(_reader_box).handle_all().empty_timeout(1ms),
		[&](DigitalInput msg)
		{
			_inputs[msg.pin] = msg;
			
			pinMode(msg.pin, INPUT);
			switch (msg.resistor_state)
			{
			case resistor::off:
				pullUpDnControl(msg.pin, PUD_OFF);
				break;
			case resistor::pull_down:
				pullUpDnControl(msg.pin, PUD_DOWN);
				break;
			case resistor::pull_up:
				pullUpDnControl(msg.pin, PUD_UP);
				break;
			}
		},
		[&](Interface::pause_interface pause)
		{
			_check_inputs = false;
		},
		[&](Interface::continue_interface cont)
		{
			_check_inputs = true;
		});
}

void DigitalInputStateReader::data_read_thread()
{
	int res = 0;
	while (stop_requested() == false)
	{
		if (_check_inputs)
		{
			for (auto& inp : _inputs)
			{
				res = digitalRead(inp.second.pin);
				if (res != inp.second.state)
				{
					inp.second.state = res;
					if (inp.second.is_inverted)
						so_5::send<DigitalMessages::return_digital_in_state>(_parent, Interface::GPIO_WIRING, inp.second.pin, !res, std::chrono::high_resolution_clock::now());
					else
						so_5::send<DigitalMessages::return_digital_in_state>(_parent, Interface::GPIO_WIRING, inp.second.pin, res, std::chrono::high_resolution_clock::now());
				}
			}
		}
		
		box_msg_handler();
		//std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}