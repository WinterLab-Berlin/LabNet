#pragma once

#include <so_5/all.hpp>
#include <gattlib.h>
#include <string>
#include <logging_facility.h>
#include <map>

#include "../../network/LabNetClient.pb.h"

namespace LabNet::interface::ble_uart
{
    struct InitBleUart
    {
        const std::string device;
        const so_5::mbox_t mbox;
    };

    class BleUartMain final : public so_5::agent_t
    {
    public:
        BleUartMain(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, log::Logger logger);
        ~BleUartMain();

    private:
        void so_define_agent() override;
        void so_evt_start() override;
        void so_evt_finish() override;

        log::Logger m_logger;
        const so_5::mbox_t m_events_box;
        const so_5::mbox_t m_interfaces_manager_box;
        const so_5::mbox_t m_self_box;

        // mbox for each device
        std::map<std::string, so_5::mbox_t> m_devices;
        // coop for each device
        std::map<std::string, so_5::coop_handle_t> m_coop;
    };
};