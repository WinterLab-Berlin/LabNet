#pragma once

#include "Interfaces.h"
#include <chrono>
#include <so_5/all.hpp>
#include <string>

namespace Interface
{
    struct stop_interface
    {
    };
    struct pause_interface
    {
    };
    struct continue_interface
    {
    };

    struct interface_init_result
    {
        Interface::Interfaces interface;
        bool is_succeed;
    };

    struct interface_lost
    {
        Interface::Interfaces interface;
    };

    struct interface_reconnected
    {
        Interface::Interfaces interface;
    };
}