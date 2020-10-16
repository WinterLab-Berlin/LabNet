#pragma once

#include "../Interface/Interfaces.h"
#include <LoggingFacility.h>
#include <so_5/all.hpp>

namespace DigitalOut
{
    struct stop_helper
    {
    };
    struct start_pulse
    {
        unsigned int high_duration;
        unsigned int low_duration;
        unsigned int pulses;
    };
    struct just_switch
    {
        bool state;
    };

    class PulseHelper final : public so_5::agent_t
    {
    public:
        PulseHelper(context_t ctx, Logger logger, so_5::mbox_t dig_out_box, so_5::mbox_t lab_net_box, so_5::mbox_t interface_box, Interface::Interfaces interface, char pin);

    private:
        void so_define_agent() override;

        so_5::state_t _runningState { this, "running" };
        so_5::state_t _startingState { this, "start" };
        so_5::state_t _waitState { this, "wait" };

        Logger _logger;
        const so_5::mbox_t _labNetBox;
        const so_5::mbox_t _interfaceBox;
        const so_5::mbox_t _digOutBox;
        char _pin;
        unsigned int _highDuration, _lowDuration, _pulses;
        so_5::timer_id_t _turnOnTimer;
        so_5::timer_id_t _turnOffTimer;
        const Interface::Interfaces _interface;
    };
}