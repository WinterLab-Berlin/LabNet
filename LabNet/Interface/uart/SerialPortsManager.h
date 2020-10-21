#pragma once

#include "../DigitalMessages.h"
#include "../InterfaceMessages.h"
#include "../StreamMessages.h"
#include "PrivateMessages.h"
#include "SerialPort.h"
#include <LoggingFacility.h>
#include <map>
#include <so_5/all.hpp>

namespace uart
{
    struct init_serial_port
    {
        Interface::Interfaces port_id;
        const int baud;
        const so_5::mbox_t mbox;
    };

    class SerialPortsManager final : public so_5::agent_t
    {
    public:
        SerialPortsManager(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, Logger logger);
        ~SerialPortsManager();

    private:
        void so_define_agent() override;
        void so_evt_start() override;
        void so_evt_finish() override;

        bool is_raspi_revision_valid();
        std::string port_name_for_id(int id);
        void get_raspi_revision();
        void init_new_port_event(const so_5::mhood_t<init_serial_port> ev);
        void try_to_reconnect_event(const so_5::mhood_t<uart::private_messages::try_to_reconnect> ev);
        void stop_interface_event(mhood_t<Interface::stop_interface> msg);

        so_5::state_t running_state { this, "running_state" };
        so_5::state_t paused_state { this, "paused_state" };

        const so_5::mbox_t _self_box;
        const so_5::mbox_t _events_box;
        const so_5::mbox_t _interfaces_manager_box;
        Logger _logger;
        std::map<int, int> _handle_for_port;
        std::map<int, std::unique_ptr<SerialPort>> _ports;

        long long _raspi_revision;
        const long long R3BPV1_3 = 0xa020d3;
        const long long R3BV1_2 = 0xa02082;
    };
}