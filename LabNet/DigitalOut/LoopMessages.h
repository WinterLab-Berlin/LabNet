#pragma once

#include <so_5/all.hpp>

namespace DigitalOut
{
    struct loop_stopped
    {
        std::string loop_name;
    };

    struct loop_start_failed
    {
        std::string loop_name;
    };
}