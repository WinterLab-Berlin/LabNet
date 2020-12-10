#pragma once

namespace LabNet::interface::uart
{
    struct DigitalOutput
    {
        uint8_t pin;
        /// has the pin value to be inverted
        bool is_inverted { false };

        DigitalOutput(uint8_t pin, bool is_inverted)
        {
            this->pin = pin;
            this->is_inverted = is_inverted;
        }
    };
};