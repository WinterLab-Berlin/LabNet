#pragma once

#include "pin_id.h"
#include <LoggingFacility.h>
#include <map>
#include <so_5/all.hpp>
#include <vector>

namespace LabNet::digital_out
{
    class DigitalOutHelper final : public so_5::agent_t
    {
    public:
        DigitalOutHelper(context_t ctx, Logger logger);
        ~DigitalOutHelper();

    private:
        void so_define_agent() override;
        void so_evt_start() override;

        so_5::state_t _runningState { this, "running" };
        so_5::state_t _pausedState { this, "paused" };

        Logger _logger;
        const so_5::mbox_t _selfBox;
        const so_5::mbox_t _server_out_box;
        const so_5::mbox_t _server_in_box;
        const so_5::mbox_t _gpioBox;
        const so_5::mbox_t _uartBox;
        const so_5::mbox_t _gpioWiringBox;
        const so_5::mbox_t _soundBox;

        std::map<std::string, so_5::mbox_t> _loopHelper;
        std::map<PinId, so_5::mbox_t> _pulseHelper;
    };
}