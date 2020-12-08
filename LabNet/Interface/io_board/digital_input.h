#pragma once

#include <stdint.h>

namespace LabNet::interface::io_board
{
    enum class Resistor
    {
        Off = 0,
        PullDown = 1,
        PullUp = 2
    };

    struct DigitalInput
    {
        /// hardware pin number (wiringPi notation)
        uint8_t pin_h;
        /// logic pin number
        uint8_t pin_l;
        /// has the pin value to be inverted
        bool is_inverted { false };
        /// current pin state
        char state { 2 };
        Resistor resistor_state { Resistor::Off };

        DigitalInput(uint8_t pin_h, uint8_t pin_l, bool is_inverted = false, Resistor state = Resistor::Off)
        {
            this->pin_h = pin_h;
            this->pin_l = pin_l;
            this->is_inverted = is_inverted;
            this->resistor_state = state;
        }

        DigitalInput(const DigitalInput& di)
        {
            pin_h = di.pin_h;
            pin_l = di.pin_l;
            is_inverted = di.is_inverted;
            resistor_state = di.resistor_state;
        }
    };
};