#pragma once

#include <stdint.h>

namespace LabNet::interface::gpio_wiring
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
        uint8_t pin;
        /// has the pin value to be inverted
        bool is_inverted { false };
        /// current pin state
        uint8_t state { 2 };
        Resistor resistor_state { Resistor::Off };

        DigitalInput(uint8_t pin, bool is_inverted = false, Resistor state = Resistor::Off)
        {
            this->pin = pin;
            this->is_inverted = is_inverted;
            this->resistor_state = state;
        }

        DigitalInput(const DigitalInput& di)
        {
            pin = di.pin;
            is_inverted = di.is_inverted;
            resistor_state = di.resistor_state;
        }
    };
};