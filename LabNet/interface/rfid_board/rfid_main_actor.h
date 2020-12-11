#pragma once

#include <vector>
#include "data_read_worker.h"
#include "max_device.h"
#include <logging_facility.h>
#include <so_5/all.hpp>
#include "../resources/resource.h"

namespace LabNet::interface::rfid_board
{
    struct InitRfidBoard
    {
        uint32_t antenna_phase1 = 0xFFFFFFFF;
        uint32_t antenna_phase2 = 0xFFFFFFFF;
        uint32_t phase_duration = 250;
        bool is_inverted = false;
        const so_5::mbox_t mbox;
    };

    struct SetPhaseMatrix
    {
        uint32_t antenna_phase1;
        uint32_t antenna_phase2;
        uint32_t phase_duration;
    };

    class RfidBoardMainActor final : public so_5::agent_t
    {
    public:
        RfidBoardMainActor(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, log::Logger logger);
        ~RfidBoardMainActor();

    private:
        void so_define_agent() override;
        void so_evt_start() override;
        void so_evt_finish() override;

        so_5::state_t init_state_ { this, "init_state" };
        so_5::state_t running_state_ { this, "running_state" };
        so_5::state_t paused_state_ { this, "paused_state" };

        std::shared_ptr<MAXDevice> device_;
        std::unique_ptr<DataReadWorker> worker_;
        log::Logger logger_;
        const so_5::mbox_t self_box_;
        const so_5::mbox_t events_box_;
        const so_5::mbox_t interfaces_manager_box_;

        uint32_t phase1_, phase2_, phase_dur_;
        bool is_inverted_;

        const so_5::mbox_t res_box_;
        const std::vector<int32_t> pins_ = {16, 28, 12, 13, 14, 10, 11, 15, 9, 5, 23, 24, 22, 4, 1};
        std::vector<resources::Resource> resources_;
        bool res_reserved_;
    };
}