#pragma once

#include "max_device.h"
#include <chrono>
#include <future>
#include <so_5/all.hpp>
#include <thread>

namespace LabNet::interface::rfid_board
{
    class DataReadWorker
    {
    public:
        DataReadWorker(std::shared_ptr<MAXDevice> dev);
        ~DataReadWorker();

    private:
        std::shared_ptr<MAXDevice> device_;
        std::thread readWorker_;
        std::promise<void> exitSignal_;
        std::future<void> futureObj_;

        bool StopRequested();

        void ReadRfidThread();
    };
}