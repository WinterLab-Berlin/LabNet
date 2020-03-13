#pragma once

#include <so_5/all.hpp>
#include <map>
#include <LoggingFacility.h>
#include "SerialPortMessages.h"
#include "SerialPort.h"

class SerialPortsManager final : public so_5::agent_t
{
public:
	SerialPortsManager(context_t ctx, Logger logger);
	~SerialPortsManager();
	
	void so_define_agent() override;
	void so_evt_start() override;
	
private:
	std::string port_name_for_id(int id);
	void get_raspi_revision();
	void init_new_port_event(const init_port& ev);
	void port_unexpected_closed_event(const port_unexpected_closed& ev);
	void new_data_from_port_event(const new_data_from_port& data);
	
	Logger m_logger;
	std::map<int, int> m_handle_for_port;
	std::map<int, int> m_port_for_handle;
	std::map<int, std::unique_ptr<SerialPort>> m_ports;
	long long m_raspiRevision;
	const long long R3BPV1_3 = 0xa020d3;
};