#pragma once

#include <logging_facility.h>
#include <so_5/all.hpp>
#include "../resources/resource.h"
#include "max_uart_board.h"
#include "data_worker.h"

namespace LabNet::interface::uart_board
{
    struct InitUartBoard
    {
        uint32_t baud = 9600;
        bool is_inverted = false;
    };

    class UartBordMainActor final : public so_5::agent_t
    {
    private:
        so_5::state_t init_state_ { this, "init_state" };
        so_5::state_t running_state_ { this, "running_state" };
        so_5::state_t paused_state_ { this, "paused_state" };

        std::unique_ptr<DataWorker> worker_;
        std::shared_ptr<MaxUartBoard> device_;
        log::Logger logger_;
        const so_5::mbox_t self_box_;
        const so_5::mbox_t events_box_;
        const so_5::mbox_t interfaces_manager_box_;
        so_5::mchain_t data_worker_box_;

        const so_5::mbox_t res_box_;
        const std::vector<int32_t> pins_ = {16, 28, 12, 13, 14, 10, 11, 15, 9, 5, 23, 24, 22, 4, 1};
        std::vector<resources::Resource> resources_;
        bool res_reserved_;

        bool is_inverted_;
        uint32_t baud_;

        void so_define_agent() override;
        void so_evt_start() override;
        void so_evt_finish() override;

    public:
        UartBordMainActor(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, log::Logger logger);
        ~UartBordMainActor();
    };
}