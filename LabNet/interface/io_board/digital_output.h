#pragma once

namespace LabNet::interface::io_board
{
    struct DigitalOutput
    {
        uint8_t pin_h;
        uint8_t pin_l;
        bool is_inverted { false };

        DigitalOutput(uint8_t pin_h, uint8_t pin_l, bool is_inverted = false)
        {
            this->pin_h = pin_h;
            this->pin_l = pin_l;
            this->is_inverted = is_inverted;
        }

        DigitalOutput(const DigitalOutput& d)
        {
            pin_h = d.pin_h;
            pin_l = d.pin_l;
            is_inverted = d.is_inverted;
        }
    };
}