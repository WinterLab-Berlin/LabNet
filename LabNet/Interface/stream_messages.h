#pragma once

#include "interfaces.h"
#include <chrono>
#include <so_5/all.hpp>

namespace LabNet::interface::stream_messages
{
    struct NewDataFromPort
    {
        Interfaces interface;
        const char pin;
        std::shared_ptr<std::vector<char>> data;
        std::chrono::time_point<std::chrono::high_resolution_clock> time;
    };

    struct SendDataComplete
    {
        Interfaces interface;
        const char pin;
    };
}