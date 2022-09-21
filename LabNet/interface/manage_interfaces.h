#pragma once

#include <logging_facility.h>
#include <so_5/all.hpp>
#include <map>

namespace LabNet::interface
{
    class ManageInterfaces final : public so_5::agent_t
    {
    public:
        ManageInterfaces(context_t ctx, log::Logger logger);

    private:
        void so_define_agent() override;
        void so_evt_start() override;

        bool is_reset_done();

        so_5::state_t running_state_ { this, "running_state" };
        so_5::state_t reset_state_ { this, "reset_state" };

        log::Logger logger_;
        const so_5::mbox_t self_mbox_;
        const so_5::mbox_t server_out_box_;
        const so_5::mbox_t server_in_box_;
        so_5::mbox_t reset_response_box_;

        so_5::mbox_t gpio_box_;
        so_5::coop_handle_t gpio_coop_;
        so_5::mbox_t rfid_board_box_;
        so_5::coop_handle_t rfid_board_coop_;
        so_5::mbox_t gpio_wiring_box_;
        so_5::coop_handle_t gpio_wiring_coop_;
        so_5::mbox_t uart_box_;
        so_5::coop_handle_t uart_coop_;
        so_5::mbox_t sound_box_;
        so_5::coop_handle_t sound_coop_;
        so_5::mbox_t chi_bio_box_;
        so_5::coop_handle_t chi_bio_coop_;
        so_5::mbox_t ble_uart_box_;
        so_5::coop_handle_t ble_uart_coop_;
        so_5::mbox_t uart_board_box_;
        so_5::coop_handle_t uart_board_coop_;

        std::map<uint8_t, bool> reset_states_;
    };
}