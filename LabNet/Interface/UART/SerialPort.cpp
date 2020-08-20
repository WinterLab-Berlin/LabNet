#include "SerialPort.h"
#include <wiringSerial.h>
#include <wiringPi.h>
#include <chrono>
#include <SerialPortMessages.h>
#include "PrivateMessages.h"
#include "../StreamMessages.h"
#include "../DigitalMessages.h"
#include <sys/ioctl.h>

using namespace uart::private_messages;

SerialPort::SerialPort(const so_5::mbox_t parent, const so_5::mchain_t sendToPortBox, const so_5::mbox_t streamDataBox, const int portId, const int portHandler, const int baud)
	: _parent(parent)
	, _portId(portId)
	, _baud(baud)
	, _sendToPortBox(sendToPortBox)
	, _streamDataBox(streamDataBox)
	, _portHandler(portHandler)
	, _futureObj(_exitSignal.get_future())
	, _isActive(true)
	, _isPinInverted(false)
{
	if (_portId == 0)
	{
		pinMode(EN_UART0, OUTPUT);
		digitalWrite(EN_UART0, 0);
	}
	
	std::thread sendWorker{&SerialPort::data_send_thread, this, _sendToPortBox };
	_sendWorker = std::move(sendWorker);
		
	std::thread readWorker{&SerialPort::data_read_thread, this};
	_readWorker = std::move(readWorker);
}

SerialPort::~SerialPort()
{
	_exitSignal.set_value();
	_sendToPortBox->close(so_5::mchain_props::close_mode_t::drop_content);
	_sendWorker.join();
	_readWorker.join();
}

void SerialPort::send_data(std::shared_ptr<std::vector<char>> data)
{
	so_5::send<std::shared_ptr<std::vector<char>>>(_sendToPortBox, data);
}

void SerialPort::data_send_thread(so_5::mchain_t ch)
{
	// write data messages until mchain will be closed.
	receive(from(ch).handle_all(), 
		[&](std::shared_ptr<std::vector<char>> data) {
			if (_isActive)
			{
				for (int i = 0; i < data->size(); i++)
				{
					serialPutchar(_portHandler, data->at(i));
				}
			
				so_5::send<uart::private_messages::send_data_complete>(_parent, _portId);
			}
		});
}
	
bool SerialPort::stop_requested()
{
	// checks if value in future object is available
	if(_futureObj.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
	    return false;
	return true;
}
	
void SerialPort::data_read_thread()
{
	while (stop_requested() == false)
	{
		if (_isActive)
		{
			int c = serialDataAvail(_portHandler);
		
			if (c > 0)
			{
				std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>();
				do
				{
					c = serialGetchar(_portHandler);
					data->push_back(c);
				} while (serialDataAvail(_portHandler));
			
				if (c < 0)
				{
					so_5::send<uart::private_messages::port_unexpected_closed>(_parent, _portId);
			
					break;
				}
			
				so_5::send<StreamMessages::new_data_from_port>(_streamDataBox, static_cast<Interface::Interfaces>(_portId + 100), 0, data, std::chrono::high_resolution_clock::now());
			}
			else if (c < 0)
			{
				so_5::send<uart::private_messages::port_unexpected_closed>(_parent, _portId, _baud);
			
				break;
			}
		}
			
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void SerialPort::set_digital_out(so_5::mbox_t report, char pin, bool state)
{
	if (pin == 1)
	{
		if (_portId == 0)
		{
			digitalWrite(EN_UART0, state);
		}
		else
		{
			int rts = 0;
			if (state)
			{
				if (ioctl(_portHandler, TIOCMGET, &rts) == -1)
				{
					// error
				}
			
				rts &= ~TIOCM_RTS;

				if (ioctl(_portHandler, TIOCMSET, &rts) == -1) 
				{
					// error
				}
			}
			else
			{
				if (ioctl(_portHandler, TIOCMGET, &rts) != -1)
				{
					// error
				}
			
				rts |= TIOCM_RTS;
				if (ioctl(_portHandler, TIOCMSET, &rts) == -1) 
				{
					// error
				}
			}
		}
		
		//_logger->writeInfoEntry(string_format("gpio set %d", msg->state));
		so_5::send<DigitalMessages::return_digital_out_state>(report, static_cast<Interface::Interfaces>(_portId + 100), pin, state, std::chrono::high_resolution_clock::now());
	}
	else
	{
		so_5::send<DigitalMessages::invalid_digital_out_pin>(report, static_cast<Interface::Interfaces>(_portId + 100), pin);
	}
}