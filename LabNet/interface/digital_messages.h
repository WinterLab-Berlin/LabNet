#pragma once

#include "interfaces.h"
#include <chrono>
#include <so_5/all.hpp>

namespace LabNet::interface::digital_messages
{
    struct DigitalInInitResult
    {
        Interfaces interface;
        const uint32_t pin;
        bool is_succeed;
    };

    struct ReturnDigitalInState
    {
        Interfaces interface;
        const uint32_t pin;
        const bool state;
        std::chrono::time_point<std::chrono::high_resolution_clock> time;
    };

    struct DigitalOutInitResult
    {
        Interfaces interface;
        const uint32_t pin;
        bool is_succeed;
    };

    struct SetDigitalOut
    {
        Interfaces interface;
        const uint32_t pin;
        const bool state;
        const so_5::mbox_t mbox;
    };

    struct ReturnDigitalOutState
    {
        Interfaces interface;
        const uint32_t pin;
        const bool state;
        std::chrono::time_point<std::chrono::high_resolution_clock> time;
    };

    struct InvalidDigitalOutPin
    {
        Interfaces interface;
        const uint32_t pin;
    };
}