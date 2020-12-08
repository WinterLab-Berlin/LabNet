#pragma once

#include "data_read_worker.h"
#include "max_device.h"
#include <LoggingFacility.h>
#include <so_5/all.hpp>

namespace LabNet::interface::rfid_board
{
    struct init_interface
    {
        uint32_t antenna_phase1 = 0xFFFFFFFF;
        uint32_t antenna_phase2 = 0xFFFFFFFF;
        uint32_t phase_duration = 250;
        bool is_inverted = false;
        const so_5::mbox_t mbox;
    };

    struct set_phase_matrix
    {
        uint32_t antenna_phase1;
        uint32_t antenna_phase2;
        uint32_t phase_duration;
    };

    class RfidBoardMainActor final : public so_5::agent_t
    {
    public:
        RfidBoardMainActor(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, Logger logger);
        ~RfidBoardMainActor();

    private:
        void so_define_agent() override;
        void so_evt_start() override;
        void so_evt_finish() override;

        so_5::state_t init_state { this, "init_state" };
        so_5::state_t running_state { this, "running_state" };
        so_5::state_t paused_state { this, "paused_state" };

        std::shared_ptr<MAXDevice> _device;
        std::unique_ptr<DataReadWorker> _worker;
        Logger _logger;
        const so_5::mbox_t _self_box;
        const so_5::mbox_t _events_box;
        const so_5::mbox_t _interfaces_manager_box;

        uint32_t _phase1, _phase2, _phase_dur;
        bool _is_inverted;
    };
}