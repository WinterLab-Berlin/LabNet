#pragma once

#include <logging_facility.h>
#include <so_5/all.hpp>
#include "../resources/resource.h"
#include <array>
#include <map>

namespace LabNet::interface::chi_bio
{
    struct InitChiBio
    {
        //const so_5::mbox_t mbox;
    };

    class ChiBioMainActor final : public so_5::agent_t
    {
    public:
        ChiBioMainActor(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, log::Logger logger);
        ~ChiBioMainActor();

    private:
        void so_define_agent() override;
        void so_evt_start() override;
        void so_evt_finish() override;

        int SetPumpFreq(int freq);
        void SetPumpDuty(std::string& pumpId, std::string& dir, float duty);
        int SelectChiBioBoard(int boardNbr);

        so_5::state_t init_state_ { this, "init_state" };
        so_5::state_t running_state_ { this, "running_state" };
        so_5::state_t paused_state_ { this, "paused_state" };

        log::Logger logger_;
        const so_5::mbox_t events_box_;
        const so_5::mbox_t interfaces_manager_box_;
        const so_5::mbox_t self_box_;

        const so_5::mbox_t res_box_;
        std::vector<resources::Resource> resources_;
        bool res_reserved_;

        std::map<std::string, std::map<std::string, std::map<std::string, int>>> registers_;
        std::array<std::string, 4> pumps_;
        std::string in1_, in2_;
        int mux_handle_;
        int pump_handle_;
        std::map<uint8_t, std::pair<uint8_t, uint32_t>> pump_stat_;
    };
}