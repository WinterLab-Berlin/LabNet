#pragma once

#include "DigitalInput.h"
#include <LoggingFacility.h>
#include <future>
#include <map>
#include <so_5/all.hpp>
#include <thread>

namespace gpio_wiring
{
    class DigitalInputStateReader
    {

    public:
        DigitalInputStateReader(const so_5::mbox_t parent, const so_5::mchain_t reader_box, Logger logger);
        ~DigitalInputStateReader();

    private:
        void state_read_thread();

        bool stop_requested();

        void data_read_thread();

        void box_msg_handler();

        std::map<char, DigitalInput> _inputs;
        const so_5::mbox_t _parent;
        const so_5::mchain_t _reader_box;
        bool _check_inputs;

        Logger _logger;
        std::thread _read_worker;
        std::promise<void> _exit_signal;
        std::future<void> _future_obj;
    };
}