#pragma once

#include "pin_id.h"
#include <logging_facility.h>
#include <map>
#include <so_5/all.hpp>
#include <vector>

namespace LabNet::digital_out
{
    class DigitalOutHelper final : public so_5::agent_t
    {
    public:
        DigitalOutHelper(context_t ctx, log::Logger logger);
        ~DigitalOutHelper();

    private:
        void so_define_agent() override;
        void so_evt_start() override;

        so_5::state_t running_state_ { this, "running" };
        so_5::state_t paused_state_ { this, "paused" };

        log::Logger logger_;
        const so_5::mbox_t self_box_;
        const so_5::mbox_t server_out_box_;
        const so_5::mbox_t server_in_box_;
        const so_5::mbox_t gpio_box_;
        const so_5::mbox_t uart_box_;
        const so_5::mbox_t gpio_wiring_box_;
        const so_5::mbox_t sound_box_;
        const so_5::mbox_t uart_board_box_;

        std::map<std::string, so_5::mbox_t> loop_helper_;
        std::map<PinId, so_5::mbox_t> pulse_helper_;
    };
}