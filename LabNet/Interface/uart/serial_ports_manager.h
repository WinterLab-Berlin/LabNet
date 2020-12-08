#pragma once

#include "../digital_messages.h"
#include "../interface_messages.h"
#include "../stream_messages.h"
#include "private_messages.h"
#include "serial_port.h"
#include <LoggingFacility.h>
#include <map>
#include <so_5/all.hpp>

namespace LabNet::interface::uart
{
    struct InitSerialPort
    {
        Interfaces port_id;
        const uint32_t baud;
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

        bool is_raspi_type_valid();
        std::string port_name_for_id(int id);
        void get_raspi_revision();
        void init_new_port_event(const so_5::mhood_t<InitSerialPort> ev);
        void try_to_reconnect_event(const so_5::mhood_t<private_messages::TryToReconnect> ev);
        void stop_interface_event(const mhood_t<StopInterface> msg);

        so_5::state_t running_state { this, "running_state" };
        so_5::state_t paused_state { this, "paused_state" };

        const so_5::mbox_t _self_box;
        const so_5::mbox_t _events_box;
        const so_5::mbox_t _interfaces_manager_box;
        Logger _logger;
        std::map<int, int> _handle_for_port;
        std::map<int, std::unique_ptr<SerialPort>> _ports;

        uint64_t _raspi_revision;
        uint16_t _raspi_type;
    };
}