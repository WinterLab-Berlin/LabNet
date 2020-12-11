#pragma once

#include <map>
#include <so_5/all.hpp>
#include <vector>

#include <logging_facility.h>
#include "digital_input.h"
#include "digital_input_state_reader.h"
#include "digital_output.h"


namespace LabNet::interface::gpio_wiring
{
    struct InitGpioWiring
    {
        const so_5::mbox_t mbox;
    };

    class GpioManager final : public so_5::agent_t
    {
    public:
        GpioManager(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, log::Logger logger);
        ~GpioManager();

    private:
        void so_define_agent() override;
        void so_evt_start() override;
        void so_evt_finish() override;

        so_5::state_t init_state_ { this, "init_state" };
        so_5::state_t running_state_ { this, "running_state" };
        so_5::state_t paused_state_ { this, "paused_state" };

        std::map<uint8_t, uint8_t> pins_;
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