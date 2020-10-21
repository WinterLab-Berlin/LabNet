#pragma once

#include <LoggingFacility.h>
#include <so_5/all.hpp>
#include <map>

namespace Interface
{
    class ManageInterfaces final : public so_5::agent_t
    {
    public:
        ManageInterfaces(context_t ctx, Logger logger, so_5::mbox_t labNetBox);

    private:
        void so_define_agent() override;
        void so_evt_start() override;

        bool is_reset_done();

        so_5::state_t running_state { this, "running_state" };
        so_5::state_t reset_state { this, "reset_state" };

        Logger _logger;
        const so_5::mbox_t _self_mbox;
        const so_5::mbox_t _labNetBox;
        so_5::mbox_t _gpio_box;
        so_5::coop_handle_t _gpio_coop;
        so_5::mbox_t _rfid_board_box;
        so_5::coop_handle_t _rfid_board_coop;
        so_5::mbox_t _gpio_wiring_box;
        so_5::coop_handle_t _gpio_wiring_coop;
        so_5::mbox_t _uart_box;
        so_5::coop_handle_t _uart_coop;

        std::map<uint8_t, bool> _reset_state;
    };
}