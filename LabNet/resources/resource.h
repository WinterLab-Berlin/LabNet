#pragma once

#include <stdint.h>

namespace LabNet
{
    namespace resources
    {
        enum class resource
        {
            None = -1,
            GPIO0 = 0, // ph 27
            GPIO1 = 1, // ph 28
            GPIO2 = 2, // ph 3
            GPIO3 = 3, // ph 5
            GPIO4 = 4, // ph 7
            GPIO5 = 5, // ph 29
            GPIO6 = 6, // ph 31
            GPIO7 = 7, // ph 26
            GPIO8 = 8, // ph 24
            GPIO9 = 9, // ph 21
            GPIO10 = 10, // ph 19
            GPIO11 = 11, // ph 23
            GPIO12 = 12, // ph 32
            GPIO13 = 13, // ph 33
            GPIO14 = 14, // ph 8
            GPIO15 = 15, // ph 10
            GPIO16 = 16, // ph 36
            GPIO17 = 17, // ph 11
            GPIO18 = 18, // ph 12
            GPIO19 = 19, // ph 35
            GPIO20 = 20, // ph 38
            GPIO21 = 21, // ph 40
            GPIO22 = 22, // ph 15
            GPIO23 = 23, // ph 16
            GPIO24 = 24, // ph 18
            GPIO25 = 25, // ph 22
            GPIO26 = 26, // ph 37
            GPIO27 = 27  // ph 13
        };

        int32_t resource_to_wiring(resource bcm);
        int32_t resource_to_wiring(int32_t bcm);
        resource wiring_to_resource(int32_t);
    }
}