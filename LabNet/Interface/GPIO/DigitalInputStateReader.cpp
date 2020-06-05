#include "DigitalInputStateReader.h"
#include "Messages.h"
#include <wiringPi.h>
#include <chrono>
#include "../DigitalMessages.h"

GPIO::DigitalInputStateReader::DigitalInputStateReader(const so_5::mbox_t parent, std::map<int, DigitalInput> *inputs, Logger logger)
	: _parent(parent)
	, _inputs(inputs)
	, _futureObj(_exitSignal.get_future())
	, _logger(logger)
{
	std::thread readWorker{&GPIO::DigitalInputStateReader::data_read_thread, this};
	_readWorker = std::move(readWorker);
}

GPIO::DigitalInputStateReader::~DigitalInputStateReader()
{
	_exitSignal.set_value();
	_readWorker.join();
}

bool GPIO::DigitalInputStateReader::stop_requested()
{
	// checks if value in future object is available
	if(_futureObj.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
	    return false;
	return true;
}

void GPIO::DigitalInputStateReader::data_read_thread()
{
	int res = 0;
	while (stop_requested() == false)
	{
		for (auto& inp : *_inputs)
		{
			if (inp.second.available)
			{
				res = digitalRead(inp.second.pin_h);
				if (res != inp.second.state)
				{
					_logger->writeInfoEntry("new state");
					inp.second.state = res;
					if (inp.second.is_inverted)
						so_5::send<DigitalMessages::return_digital_in_state>(_parent, Interface::GPIO_TOP_PLANE, inp.second.pin_l, !res, std::chrono::high_resolution_clock::now());
					else
						so_5::send<DigitalMessages::return_digital_in_state>(_parent, Interface::GPIO_TOP_PLANE, inp.second.pin_l, res, std::chrono::high_resolution_clock::now());
				}
			}
		}
			
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}