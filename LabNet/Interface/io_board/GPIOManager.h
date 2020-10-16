#pragma once

#include "DigitalInput.h"
#include "DigitalInputStateReader.h"
#include "DigitalOutput.h"
#include <LoggingFacility.h>
#include <map>
#include <so_5/all.hpp>

namespace io_board
{
    struct init_io_board
    {
        const so_5::mbox_t mbox;
    };

    class GPIOManager final : public so_5::agent_t
    {
    public:
        GPIOManager(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, Logger logger);
        ~GPIOManager();

    private:
        void so_define_agent() override;
        void so_evt_start() override;
        void so_evt_finish() override;

        so_5::state_t init_state { this, "init_state" };
        so_5::state_t running_state { this, "running_state" };
        so_5::state_t paused_state { this, "paused_state" };

        std::map<char, char> _outPins;
        std::map<char, char> _inPins;
        std::map<int, DigitalInput> _inputs;
        std::map<int, DigitalOutput> _outputs;
        std::unique_ptr<DigitalInputStateReader> _inputStateReader;

        Logger _logger;
        const so_5::mbox_t _events_box;
        const so_5::mbox_t _interfaces_manager_box;
        const so_5::mbox_t _self_box;
    };
}