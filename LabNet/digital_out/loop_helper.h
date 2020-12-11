#pragma once

#include "pin_id.h"
#include <logging_facility.h>
#include <chrono>
#include <so_5/all.hpp>
#include <vector>

namespace LabNet::digital_out
{
    struct AbortLoopHelper
    {
    };

    struct PauseLoopHelper
    {
    };

    struct ContinueLoopHelper
    {
    };

    struct DigitalOutputParameter
    {
        PinId id;
        uint offset;
        uint duration;
        bool handled = false;
        bool state = false;
    };

    class LoopHelper final : public so_5::agent_t
    {
    public:
        LoopHelper(context_t ctx, log::Logger logger, so_5::mbox_t dig_out_box, so_5::mbox_t report_box);
        ~LoopHelper();

    private:
        void so_define_agent() override;
        void TurnPinOn(PinId& id);
        void TurnPinOff(PinId& id);

        so_5::state_t wait_state_ { this, "wait" };
        so_5::state_t running_state_ { this, "running" };
        so_5::state_t pause_state_ { this, "pause" };

        std::string loop_name_;
        log::Logger logger_;
        const so_5::mbox_t report_box_;
        const so_5::mbox_t dig_out_box_;
        so_5::mbox_t gpio_box_;
        so_5::mbox_t gpio_wiring_box_;
        so_5::mbox_t uart_box_;

        uint32_t loop_pause_;
        std::vector<DigitalOutputParameter> loop_;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_;
    };
}