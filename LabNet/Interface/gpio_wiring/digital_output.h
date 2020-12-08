#pragma once

namespace LabNet::interface::gpio_wiring

{
    struct DigitalOutput
    {
        uint8_t pin;
        bool is_inverted { false };

        DigitalOutput(uint8_t pin, bool is_inverted = false)
        {
            this->pin = pin;
            this->is_inverted = is_inverted;
        }

        DigitalOutput(const DigitalOutput& d)
        {
            pin = d.pin;
            is_inverted = d.is_inverted;
        }
    };
}