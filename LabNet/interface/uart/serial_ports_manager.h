#pragma once

#include "../digital_messages.h"
#include "../interface_messages.h"
#include "../stream_messages.h"
#include "private_messages.h"
#include "serial_port.h"
#include <logging_facility.h>
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
        SerialPortsManager(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, log::Logger logger);
        ~SerialPortsManager();

    private:
        void so_define_agent() override;
        void so_evt_start() override;
        void so_evt_finish() override;

        bool IsRaspiTypeValid();
        std::string PortNameForId(int id);
        void GetRaspiRevision();
        void InitNewPortEvent(const so_5::mhood_t<InitSerialPort> ev);
        void TryToReconnectEvent(const so_5::mhood_t<private_messages::TryToReconnect> ev);
        void StopInterfaceEvent(const mhood_t<StopInterface> msg);

        so_5::state_t running_state_ { this, "running_state" };
        so_5::state_t paused_state_ { this, "paused_state" };

        const so_5::mbox_t self_box_;
        const so_5::mbox_t events_box_;
        const so_5::mbox_t interfaces_manager_box_;
        log::Logger logger_;
        std::map<int, int> handle_for_port_;
        std::map<int, std::unique_ptr<SerialPort>> ports_;

        uint64_t raspi_revision_;
        uint16_t raspi_type_;
    };
}