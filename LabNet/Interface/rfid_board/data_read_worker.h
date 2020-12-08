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
        std::shared_ptr<MAXDevice> _device;
        std::thread _readWorker;
        std::promise<void> _exitSignal;
        std::future<void> _futureObj;

        bool stop_requested();

        void read_rfid_thread();
    };
}