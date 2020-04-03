#include "DigitalInputStateReader.h"
#include "Messages.h"
#include <wiringPi.h>

GPIO::DigitalInputStateReader::DigitalInputStateReader(const so_5::mbox_t parent, std::map<int, DigitalInput>& inputs)
	: _parent(parent)
	, _inputs(inputs)
	, _futureObj(_exitSignal.get_future())
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
		for (auto& inp : _inputs)
		{
			if (inp.second.available)
			{
				res = digitalRead(inp.second.pin_h);
				if (res != inp.second.state)
				{
					inp.second.state = res;
					if (inp.second.is_inverted)
						so_5::send<return_digital_in_state>(_parent, inp.second.pin_l, !res);
					else
						so_5::send<return_digital_in_state>(_parent, inp.second.pin_l, res);
				}
			}
		}
			
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}