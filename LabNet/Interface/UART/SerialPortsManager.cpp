#include "SerialPortsManager.h"

#include <string>
#include <fstream>
#include <unistd.h>
#include <wiringSerial.h>

SerialPortsManager::SerialPortsManager(context_t ctx, const so_5::mbox_t mbox, Logger logger)
	: so_5::agent_t(ctx)
	, m_parentMbox(mbox)
	, m_logger(logger)
{
	get_raspi_revision();

	m_handle_for_port[0] = -1;
	m_handle_for_port[1] = -1;
	m_handle_for_port[2] = -1;
	m_handle_for_port[3] = -1;
	m_handle_for_port[4] = -1;
}

SerialPortsManager::~SerialPortsManager()
{
}

void SerialPortsManager::so_define_agent()
{
	so_subscribe_self()
		.event(&SerialPortsManager::init_new_port_event)
		.event(&SerialPortsManager::port_unexpected_closed_event)
		.event(&SerialPortsManager::new_data_from_port_event)
		.event(&SerialPortsManager::send_data_to_port_event)
		.event(&SerialPortsManager::try_to_reconnect_event);
}

void SerialPortsManager::init_new_port_event(const init_port& ev)
{
	if (ev.port_id > -1 && ev.port_id < 5)
	{
		if (m_handle_for_port[ev.port_id] < 0)
		{
			std::string port = port_name_for_id(ev.port_id);
			if (port.size() == 0)
			{
				so_5::send <init_port_error>(m_parentMbox, ev.port_id);
				return;
			}
			
			int handle = serialOpen(port.c_str(), ev.baud);
			if (handle < 0)
			{
				so_5::send <init_port_error>(m_parentMbox, ev.port_id);
			}
			else
			{
				so_5::mchain_t sendToPortBox = create_mchain(this->so_environment());
				
				m_ports[ev.port_id] = std::make_unique<SerialPort>(so_direct_mbox(), sendToPortBox, ev.port_id, handle, ev.baud);
				m_handle_for_port[ev.port_id] = handle;
				
				so_5::send <init_port_success>(m_parentMbox, ev.port_id);
			}
		}
	}
}

void SerialPortsManager::try_to_reconnect_event(const try_to_reconnect& ev)
{
	if (ev.port_id > -1 && ev.port_id < 5)
	{
		if (m_handle_for_port[ev.port_id] < 0)
		{
			std::string port = port_name_for_id(ev.port_id);
			if (port.size() == 0)
			{
				so_5::send_delayed<try_to_reconnect>(so_direct_mbox(), std::chrono::seconds(1), ev.port_id, ev.baud);	
				return;
			}
			
			int handle = serialOpen(port.c_str(), ev.baud);
			if (handle < 0)
			{
				so_5::send_delayed<try_to_reconnect>(so_direct_mbox(), std::chrono::seconds(1), ev.port_id, ev.baud);
			}
			else
			{
				so_5::mchain_t sendToPortBox = create_mchain(this->so_environment());
				
				m_ports[ev.port_id] = std::make_unique<SerialPort>(so_direct_mbox(), sendToPortBox, ev.port_id, handle, ev.baud);
				m_handle_for_port[ev.port_id] = handle;
				
				// reconnected
				so_5::send <port_reconnected>(m_parentMbox, ev.port_id);
			}
		}
	}
}

void SerialPortsManager::send_data_to_port_event(const send_data_to_port& data)
{
	auto it = m_ports.find(data.port_id);
	if (it != m_ports.end())
	{
		m_ports[data.port_id]->send_data(data.data);
	}
}

void SerialPortsManager::port_unexpected_closed_event(const port_unexpected_closed& ev)
{
	std::string mes("port unexpected closed serial port ");
	mes += (ev.port_id + '0');
	m_logger->writeInfoEntry(mes);
	
	m_ports.erase(ev.port_id);
	m_handle_for_port[ev.port_id] = -1;
	
	so_5::send_delayed<try_to_reconnect>(so_direct_mbox(), std::chrono::seconds(1), ev.port_id, ev.baud);
}

void SerialPortsManager::new_data_from_port_event(const new_data_from_port& data)
{
	m_logger->writeInfoEntry("new data from port");
}

void SerialPortsManager::so_evt_start()
{
	
}

std::string SerialPortsManager::port_name_for_id(int id)
{
	if (m_raspiRevision > 0)
	{
		if (id == 0)
			return std::string("/dev/ttyAMA0");
		
		char path[23] = "/sys/class/tty/ttyUSB ";
		char buffer[1024];
		for (int i = 0; i < 4; i++)
		{
			path[21] = i + '0';
			int portId = -1;
        
			/* befor Raspi3B+ V1.3 the path length from "readlink" was always 84
			 * and it was easy to identify on which USB port the UART converter connected to.
			 **/
			int len = readlink(path, buffer, sizeof(buffer) - 1);
			if (len > 0)
			{
				if (m_raspiRevision == R3BPV1_3)
				{
					if (len == 84)
					{
						if (buffer[59] == '3')
							portId = 3;
						else if (buffer[59] == '2')
							portId = 4;
					}
					else if (len == 94)
					{
						if (buffer[69] == '3')
							portId = 2;
						else if (buffer[69] == '2')
							portId = 1;
					}
				}
				else
				{
					/* for Raspi 1 until Raspi3 V1.2*/
					if (len == 84)
					{
						if (buffer[59] == '2')
							portId = 1;
						else if (buffer[59] == '3')
							portId = 2;
						else if (buffer[59] == '4')
							portId = 3;
						else if (buffer[59] == '5')
							portId = 4;
					}
				}
			}
			
			if (portId == id)
			{
				std::string portName("/dev/ttyUSB");
				portName += ('0' + i);
				
				return portName;
			}
		}
	}
	
	return "";
}

void SerialPortsManager::get_raspi_revision()
{
	using namespace std;
	
	m_raspiRevision = -1;
	string line;
	ifstream cpuinfo("/proc/cpuinfo");
	if (cpuinfo.is_open())
	{
		while (getline(cpuinfo, line))
		{
			if (line.rfind("Revision", 0) == 0)
			{
				int pos = line.rfind(':');
				if (pos != string::npos)
				{
					string rev = line.substr(pos + 2, (line.size() - pos - 2));
					std::string::size_type sz = 0;
					m_raspiRevision = std::stoll(rev, &sz, 16);
				}
			}
		}
	}
}