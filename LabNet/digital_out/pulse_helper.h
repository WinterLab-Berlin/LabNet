#pragma once

#include "../interface/interfaces.h"
#include <logging_facility.h>
#include <so_5/all.hpp>

namespace LabNet::digital_out
{
    struct StopHelper
    {
    };
    struct StartPulse
    {
        unsigned int high_duration;
        unsigned int low_duration;
        unsigned int pulses;
    };
    struct JustSwitch
    {
        bool state;
    };

    class PulseHelper final : public so_5::agent_t
    {
    public:
        PulseHelper(context_t ctx, log::Logger logger, so_5::mbox_t dig_out_box, so_5::mbox_t report_box, so_5::mbox_t interface_box, interface::Interfaces interface, uint8_t pin);

    private:
        void so_define_agent() override;

        so_5::state_t running_state_ { this, "running" };
        so_5::state_t starting_state_ { this, "start" };
        so_5::state_t wait_state_ { this, "wait" };

        log::Logger logger_;
        const so_5::mbox_t report_box_;
        const so_5::mbox_t interface_box_;
        const so_5::mbox_t dig_out_box_;
        uint8_t pin_;
        uint32_t high_duration_, low_duration_, pulses_;
        so_5::timer_id_t turn_on_timer_;
        so_5::timer_id_t turn_off_timer_;
        const interface::Interfaces interface_;
    };
}