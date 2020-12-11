#pragma once

#include "digital_input.h"
#include "digital_input_state_reader.h"
#include "digital_output.h"
#include <logging_facility.h>
#include <map>
#include <so_5/all.hpp>

namespace LabNet::interface::io_board
{
    struct InitIoBoard
    {
        const so_5::mbox_t mbox;
    };

    class BoardActor final : public so_5::agent_t
    {
    public:
        BoardActor(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, log::Logger logger);
        ~BoardActor();

    private:
        void so_define_agent() override;
        void so_evt_start() override;
        void so_evt_finish() override;

        so_5::state_t init_state_ { this, "init_state" };
        so_5::state_t running_state_ { this, "running_state" };
        so_5::state_t paused_state_ { this, "paused_state" };

        std::map<uint8_t, uint8_t> out_pins_;
        std::map<uint8_t, uint8_t> in_pins_;
        std::map<uint8_t, std::unique_ptr<DigitalInput>> inputs_;
        std::map<uint8_t, std::unique_ptr<DigitalOutput>> outputs_;
        std::unique_ptr<DigitalInputStateReader> input_state_reader_;

        log::Logger logger_;
        const so_5::mbox_t events_box_;
        const so_5::mbox_t interfaces_manager_box_;
        const so_5::mbox_t self_box_;
        so_5::mchain_t reader_box_;

        so_5::mbox_t res_helper_box_;
        so_5::coop_handle_t res_helper_coop_;
    };
}