#pragma once

#include <stdint.h>

namespace LabNet
{
    namespace resources
    {
        enum class resource
        {
            None = -1,
            GPIO0 = 0, // bcm, ph 27
            GPIO1 = 1, // bcm, ph 28
            GPIO2 = 2, // bcm, ph 3
            GPIO3 = 3, // bcm, ph 5
            GPIO4 = 4, // bcm, ph 7
            GPIO5 = 5, // bcm, ph 29
            GPIO6 = 6, // bcm, ph 31
            GPIO7 = 7, // bcm, ph 26
            GPIO8 = 8, // bcm, ph 24
            GPIO9 = 9, // bcm, ph 21
            GPIO10 = 10, // bcm, ph 19
            GPIO11 = 11, // bcm, ph 23
            GPIO12 = 12, // bcm, ph 32
            GPIO13 = 13, // bcm, ph 33
            GPIO14 = 14, // bcm, ph 8
            GPIO15 = 15, // bcm, ph 10
            GPIO16 = 16, // bcm, ph 36
            GPIO17 = 17, // bcm, ph 11
            GPIO18 = 18, // bcm, ph 12
            GPIO19 = 19, // bcm, ph 35
            GPIO20 = 20, // bcm, ph 38
            GPIO21 = 21, // bcm, ph 40
            GPIO22 = 22, // bcm, ph 15
            GPIO23 = 23, // bcm, ph 16
            GPIO24 = 24, // bcm, ph 18
            GPIO25 = 25, // bcm, ph 22
            GPIO26 = 26, // bcm, ph 37
            GPIO27 = 27 // bcm, ph 13
        };

        int32_t bcm_to_wiring(resource bcm);
        int32_t bcm_to_wiring(int32_t bcm);
        resource wiring_to_bcm(int32_t);
    }
}