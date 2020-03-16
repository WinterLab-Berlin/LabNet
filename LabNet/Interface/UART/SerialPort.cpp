#include "SerialPort.h"
#include <wiringSerial.h>
#include <chrono>
#include <SerialPortMessages.h>

SerialPort::SerialPort(const so_5::mbox_t parent, const so_5::mchain_t sendToPortBox, const int portId, const int portHandler, const int baud)
	: m_parent(parent)
	, m_portId(portId)
	, m_baud(baud)
	, m_sendToPortBox(sendToPortBox)
	, m_portHandler(portHandler)
	, m_futureObj(m_exitSignal.get_future())
{
	std::thread sendWorker{&SerialPort::data_send_thread, this, m_sendToPortBox };
	m_sendWorker = std::move(sendWorker);
		
	std::thread readWorker{&SerialPort::data_read_thread, this};
	m_readWorker = std::move(readWorker);
}

SerialPort::~SerialPort()
{
	m_exitSignal.set_value();
	m_sendToPortBox->close(so_5::mchain_props::close_mode_t::drop_content);
	m_sendWorker.join();
	m_readWorker.join();
}

void SerialPort::send_data(std::shared_ptr<std::vector<char>> data)
{
	so_5::send<std::shared_ptr<std::vector<char>>>(m_sendToPortBox, data);
}

void SerialPort::data_send_thread(so_5::mchain_t ch)
{
	// write data messages until mchain will be closed.
	receive(from(ch).handle_all(), 
		[&](std::shared_ptr<std::vector<char>> data) {
			for (int i = 0; i < data->size(); i++)
			{
				serialPutchar(m_portHandler, data->at(i));
			}
			
			so_5::send<send_data_complete>(m_parent, m_portId);
		});
}
	
bool SerialPort::stop_requested()
{
	// checks if value in future object is available
	if(m_futureObj.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
	    return false;
	return true;
}
	
void SerialPort::data_read_thread()
{
	while (stop_requested() == false)
	{
		int c = serialDataAvail(m_portHandler);
		
		if (c > 0)
		{
			std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>();
			do
			{
				c = serialGetchar(m_portHandler);
				data->push_back(c);
			} while (serialDataAvail(m_portHandler));
			
			if (c < 0)
			{
				so_5::send<port_unexpected_closed>(m_parent, m_portId);
			
				break;
			}
			
			so_5::send<new_data_from_port>(m_parent, m_portId, data);
		}
		else if (c < 0)
		{
			so_5::send<port_unexpected_closed>(m_parent, m_portId, m_baud);
			
			break;
		}
			
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
