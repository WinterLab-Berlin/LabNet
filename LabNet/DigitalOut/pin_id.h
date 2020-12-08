#pragma once

#include <../Interface/interfaces.h>
#include <map>
#include <so_5/all.hpp>

namespace LabNet::digital_out
{
    struct PinId
    {
        interface::Interfaces interface;
        unsigned int pin;

        bool operator<(const PinId& a) const
        {
            return std::tie(interface, pin) < std::tie(a.interface, a.pin);
        }
    };
}