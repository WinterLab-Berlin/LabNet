#pragma once

#include <chrono>
#include <future>
#include <so_5/all.hpp>
#include <thread>
#include "max_uart_board.h"

namespace LabNet::interface::uart_board
{
    struct SetGpioMessage
    {
        uint32_t port = 0;
        uint32_t pin = 0;
        bool state;
    };

    class DataWorker
    {
    public:
        DataWorker(std::shared_ptr<MaxUartBoard> dev, const so_5::mbox_t send_to_box, const so_5::mchain_t receive_box);
        ~DataWorker();

    private:
        std::shared_ptr<MaxUartBoard> device_;
        std::thread read_worker_;
        std::promise<void> exit_signal_;
        std::future<void> future_obj_;
        const so_5::mbox_t send_to_box_;
        const so_5::mchain_t receive_box_;

        bool StopRequested();

        void ReadRfidThread();

        void BoxMsgHandler();
    };
}