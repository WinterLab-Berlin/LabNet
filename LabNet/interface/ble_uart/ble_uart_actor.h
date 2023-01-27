#pragma once

#include <so_5/all.hpp>
#include <string>
#include <logging_facility.h>
#include "connection.h"

#include <LabNetClient.pb.h>

namespace LabNet::interface::ble_uart
{
    class BleUartActor final : public so_5::agent_t
    {
    public:
        BleUartActor(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t parent, const so_5::mbox_t stream_data_box, std::string device, const char id, log::Logger logger);
        ~BleUartActor();

    private:
        void so_define_agent() override;
        void so_evt_start() override;
        void so_evt_finish() override;

        const std::string device_;
        const char id_;
        const so_5::mbox_t self_box_;
        const so_5::mbox_t parent_;
        const so_5::mbox_t stream_data_box_;
        log::Logger logger_;

        uint8_t connection_trials_;
        bool send_data_;
        so_5::state_t init_state_ { this, "init_state" };
        so_5::state_t running_state_ { this, "running_state" };
        so_5::state_t disconnected_state_ { this, "disconnected_state" };

        Connection connection_;
    };
}
