#pragma once

#include <stdint.h>

namespace LabNet
{
    namespace resources
    {
        enum class Resource
        {
            None = -1,
            Gpio0 = 0, // ph 27
            Gpio1 = 1, // ph 28
            Gpio2 = 2, // ph 3
            Gpio3 = 3, // ph 5
            Gpio4 = 4, // ph 7
            Gpio5 = 5, // ph 29
            Gpio6 = 6, // ph 31
            Gpio7 = 7, // ph 26
            Gpio8 = 8, // ph 24
            Gpio9 = 9, // ph 21
            Gpio10 = 10, // ph 19
            Gpio11 = 11, // ph 23
            Gpio12 = 12, // ph 32
            Gpio13 = 13, // ph 33
            Gpio14 = 14, // ph 8
            Gpio15 = 15, // ph 10
            Gpio16 = 16, // ph 36
            Gpio17 = 17, // ph 11
            Gpio18 = 18, // ph 12
            Gpio19 = 19, // ph 35
            Gpio20 = 20, // ph 38
            Gpio21 = 21, // ph 40
            Gpio22 = 22, // ph 15
            Gpio23 = 23, // ph 16
            Gpio24 = 24, // ph 18
            Gpio25 = 25, // ph 22
            Gpio26 = 26, // ph 37
            Gpio27 = 27  // ph 13
        };

        int32_t ResourceToWiring(Resource bcm);
        int32_t ResourceToWiring(int32_t bcm);
        Resource WiringToResource(int32_t);
    }
}