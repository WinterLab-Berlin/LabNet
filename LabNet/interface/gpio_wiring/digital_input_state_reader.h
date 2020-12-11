#pragma once

#include "digital_input.h"
#include <logging_facility.h>
#include <future>
#include <map>
#include <so_5/all.hpp>
#include <thread>

namespace LabNet::interface::gpio_wiring
{
    class DigitalInputStateReader
    {

    public:
        DigitalInputStateReader(const so_5::mbox_t parent, const so_5::mchain_t reader_box, log::Logger logger);
        ~DigitalInputStateReader();

    private:
        bool stop_requested();

        void data_read_thread();

        void box_msg_handler();

        std::map<uint8_t, std::shared_ptr<DigitalInput>> inputs_;
        const so_5::mbox_t parent_;
        const so_5::mchain_t reader_box_;
        bool check_inputs_;

        log::Logger logger_;
        std::thread read_worker_;
        std::promise<void> exit_signal_;
        std::future<void> future_obj_;
    };
}