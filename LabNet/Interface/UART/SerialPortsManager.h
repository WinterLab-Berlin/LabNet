#pragma once

#include <so_5/all.hpp>
#include <map>
#include <LoggingFacility.h>
#include "SerialPortMessages.h"
#include "SerialPort.h"
#include "PrivateMessages.h"
#include "../InterfaceMessages.h"
#include "../StreamMessages.h"
#include "../DigitalMessages.h"

namespace uart
{
	class SerialPortsManager final : public so_5::agent_t
	{
	public:
		SerialPortsManager(context_t ctx, const so_5::mbox_t selfBox, const so_5::mbox_t mbox, Logger logger);
		~SerialPortsManager();
	
		void so_define_agent() override;
		void so_evt_start() override;
	
	private:
		std::string port_name_for_id(int id);
		void get_raspi_revision();
		void init_new_port_event(const uart::messages::init_port& ev);
		void try_to_reconnect_event(const uart::private_messages::try_to_reconnect& ev);
		void port_unexpected_closed_event(const uart::private_messages::port_unexpected_closed& ev);
		void send_data_to_port_event(const StreamMessages::send_data_to_port& data);
		void send_data_complete_event(const uart::private_messages::send_data_complete& mes);
		void pause_interface_event(const Interface::pause_interface &mes);
		void reset_interface_event(const Interface::reset_interface &mes);
		void set_digital_out_event(const DigitalMessages::set_digital_out &mes);
		void continue_interface_event(const Interface::continue_interface &mes);
	
		const so_5::mbox_t _selfBox;
		const so_5::mbox_t _parentMbox;
		Logger _logger;
		std::map<int, int> _handle_for_port;
		std::map<int, std::unique_ptr<SerialPort>> _ports;
		
		long long _raspiRevision;
		const long long R3BPV1_3 = 0xa020d3;
		const long long R3BV1_2 = 0xa02082;
	};
}