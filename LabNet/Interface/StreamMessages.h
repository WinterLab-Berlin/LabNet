#pragma once

#include "Interfaces.h"
#include <chrono>
#include <so_5/all.hpp>

namespace StreamMessages
{
    struct new_data_from_port
    {
        Interface::Interfaces interface;
        const char pin;
        std::shared_ptr<std::vector<char>> data;
        std::chrono::time_point<std::chrono::high_resolution_clock> time;
    };

    struct send_data_complete
    {
        Interface::Interfaces interface;
        const char pin;
    };
}