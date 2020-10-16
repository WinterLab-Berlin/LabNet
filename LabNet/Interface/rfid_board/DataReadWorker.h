#pragma once

#include "MAXDevice.h"
#include <chrono>
#include <future>
#include <so_5/all.hpp>
#include <thread>

namespace MAX14830
{
    class DataReadWorker
    {
    public:
        DataReadWorker(std::shared_ptr<MAX14830::MAXDevice> dev);
        ~DataReadWorker();

    private:
        std::shared_ptr<MAX14830::MAXDevice> _device;
        std::thread _readWorker;
        std::promise<void> _exitSignal;
        std::future<void> _futureObj;

        bool stop_requested();

        void read_rfid_thread();
    };
}