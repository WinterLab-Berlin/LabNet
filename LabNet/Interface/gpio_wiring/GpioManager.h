#pragma once

#include "DigitalInput.h"
#include "DigitalInputStateReader.h"
#include "DigitalOutput.h"
#include <LoggingFacility.h>
#include <map>
#include <so_5/all.hpp>
#include <vector>

namespace gpio_wiring
{
    struct init_gpio_wiring
    {
        const so_5::mbox_t mbox;
    };

    class GpioManager final : public so_5::agent_t
    {
    public:
        GpioManager(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, Logger logger);
        ~GpioManager();

    private:
        void so_define_agent() override;
        void so_evt_start() override;
        void so_evt_finish() override;

        so_5::state_t init_state { this, "init_state" };
        so_5::state_t running_state { this, "running_state" };
        so_5::state_t paused_state { this, "paused_state" };

        std::map<char, char> _pins;
        std::map<int, DigitalOutput> _outputs;
        std::unique_ptr<DigitalInputStateReader> _inputStateReader;

        Logger _logger;
        const so_5::mbox_t _events_box;
        const so_5::mbox_t _interfaces_manager_box;
        const so_5::mbox_t _self_box;
        so_5::mchain_t _reader_box;
    };
}