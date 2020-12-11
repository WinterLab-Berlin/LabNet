#pragma once

namespace LabNet::interface::uart
{
    struct DigitalInput
    {
        uint8_t pin;
        /// has the pin value to be inverted
        bool is_inverted { false };
        /// current pin state
        uint8_t state { 2 };

        DigitalInput(uint8_t pin, bool is_inverted)
        {
            this->pin = pin;
            this->is_inverted = is_inverted;
        }
    };
};