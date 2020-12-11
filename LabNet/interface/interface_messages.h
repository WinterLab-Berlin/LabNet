#pragma once

#include "interfaces.h"
#include <chrono>
#include <so_5/all.hpp>
#include <string>

namespace LabNet::interface
{
    struct PauseInterface
    {
    };
    struct ContinueInterface
    {
    };
    struct StopInterface
    {
    };

    struct InterfaceInitResult
    {
        Interfaces interface;
        bool is_succeed;
    };

    struct InterfaceStopped
    {
        Interfaces interface;
    };

    struct InterfaceLost
    {
        Interfaces interface;
    };

    struct InterfaceReconnected
    {
        Interfaces interface;
    };
}