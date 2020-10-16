#pragma once

namespace io_board
{
    struct DigitalInput
    {
        /// hardware pin number (wiringPi notation)
        char pin_h;
        /// logic pin number
        char pin_l;
        /// has the pin value to be inverted
        bool is_inverted { false };
        ///
        bool available { false };
        /// current pin state
        char state { 2 };
    };
}