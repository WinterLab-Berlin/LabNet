#include "SerialPortsManager.h"

#include <string>
#include <fstream>
#include <unistd.h>
#include <wiringSerial.h>

using namespace uart::private_messages;

uart::SerialPortsManager::SerialPortsManager(context_t ctx, const so_5::mbox_t selfBox, const so_5::mbox_t mbox, Logger logger)
	: so_5::agent_t(ctx)
	, _selfBox(selfBox)
	, _parentMbox(mbox)
	, _logger(logger)
{
	get_raspi_revision();

	_handle_for_port[0] = -1;
	_handle_for_port[1] = -1;
	_handle_for_port[2] = -1;
	_handle_for_port[3] = -1;
	_handle_for_port[4] = -1;
}

uart::SerialPortsManager::~SerialPortsManager()
{
}

void uart::SerialPortsManager::so_define_agent()
{
	so_default_state()
		.event(_selfBox, &SerialPortsManager::init_new_port_event)
		.event(_selfBox, &SerialPortsManager::port_unexpected_closed_event)
		.event(_selfBox, &SerialPortsManager::new_data_from_port_event)
		.event(_selfBox, &SerialPortsManager::send_data_to_port_event)
		.event(_selfBox, &SerialPortsManager::try_to_reconnect_event)
		.event(_selfBox, &SerialPortsManager::pause_interface_event)
		.event(_selfBox, &SerialPortsManager::reset_interface_event)
		.event(_selfBox, &SerialPortsManager::continue_interface_event)
		.event(_selfBox, &SerialPortsManager::send_data_complete_event);
}

void uart::SerialPortsManager::init_new_port_event(const uart::messages::init_port& ev)
{
	if (ev.port_id > -1 && ev.port_id < 5)
	{
		if (_handle_for_port[ev.port_id] < 0)
		{
			std::string port = port_name_for_id(ev.port_id);
			if (port.size() == 0)
			{
				so_5::send <uart::messages::init_port_result>(_parentMbox, ev.port_id, false);
				return;
			}
			
			int handle = serialOpen(port.c_str(), ev.baud);
			if (handle < 0)
			{
				so_5::send <uart::messages::init_port_result>(_parentMbox, ev.port_id, false);
			}
			else
			{
				so_5::mchain_t sendToPortBox = create_mchain(this->so_environment());
				
				_ports[ev.port_id] = std::make_unique<SerialPort>(_selfBox, sendToPortBox, ev.port_id, handle, ev.baud);
				_handle_for_port[ev.port_id] = handle;
				
				so_5::send <uart::messages::init_port_result>(_parentMbox, ev.port_id, true);
			}
		}
	}
}

void uart::SerialPortsManager::try_to_reconnect_event(const try_to_reconnect& ev)
{
	if (ev.port_id > -1 && ev.port_id < 5)
	{
		if (_handle_for_port[ev.port_id] < 0)
		{
			std::string port = port_name_for_id(ev.port_id);
			if (port.size() == 0)
			{
				so_5::send_delayed<try_to_reconnect>(_selfBox, std::chrono::seconds(1), ev.port_id, ev.baud);	
				return;
			}
			
			int handle = serialOpen(port.c_str(), ev.baud);
			if (handle < 0)
			{
				so_5::send_delayed<try_to_reconnect>(_selfBox, std::chrono::seconds(1), ev.port_id, ev.baud);
			}
			else
			{
				so_5::mchain_t sendToPortBox = create_mchain(this->so_environment());
				
				_ports[ev.port_id] = std::make_unique<SerialPort>(_selfBox, sendToPortBox, ev.port_id, handle, ev.baud);
				_handle_for_port[ev.port_id] = handle;
				
				// reconnected
				so_5::send <uart::messages::port_reconnected>(_parentMbox, ev.port_id);
			}
		}
	}
}

void uart::SerialPortsManager::send_data_to_port_event(const uart::messages::send_data_to_port& data)
{
	auto it = _ports.find(data.port_id);
	if (it != _ports.end())
	{
		_ports[data.port_id]->send_data(data.data);
	}
}

void uart::SerialPortsManager::send_data_complete_event(const uart::messages::send_data_complete)
{
	so_5::send<uart::messages::send_data_complete>(_parentMbox);
}

void uart::SerialPortsManager::port_unexpected_closed_event(const uart::messages::port_unexpected_closed& ev)
{
	_logger->writeInfoEntry(string_format("port unexpected closed serial port %d", ev.port_id));
	
	_ports.erase(ev.port_id);
	_handle_for_port[ev.port_id] = -1;
	
	so_5::send <uart::messages::port_unexpected_closed>(_parentMbox, ev);
	so_5::send_delayed<try_to_reconnect>(_selfBox, std::chrono::seconds(1), ev.port_id, ev.baud);
}

void uart::SerialPortsManager::new_data_from_port_event(const uart::messages::new_data_from_port& data)
{
	so_5::send<uart::messages::new_data_from_port>(_parentMbox, data);
}

void uart::SerialPortsManager::pause_interface_event(const Interface::pause_interface &mes)
{
	for (auto& port : _ports)
	{
		port.second->deactivate_send_receive();
	}
}

void uart::SerialPortsManager::reset_interface_event(const Interface::reset_interface &mes)
{
	for (auto& handle : _handle_for_port)
		handle.second = -1;
	
	_ports.clear();
}

void uart::SerialPortsManager::continue_interface_event(const Interface::continue_interface &mes)
{
	for (auto& port : _ports)
	{
		port.second->activate_send_receive();
	}
}

void uart::SerialPortsManager::so_evt_start()
{
	
}

std::string uart::SerialPortsManager::port_name_for_id(int id)
{
	if (_raspiRevision > 0)
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
				if (_raspiRevision == R3BPV1_3)
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

void uart::SerialPortsManager::get_raspi_revision()
{
	using namespace std;
	
	_raspiRevision = -1;
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
					_raspiRevision = std::stoll(rev, &sz, 16);
				}
			}
		}
	}
}