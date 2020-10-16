#pragma once

namespace gpio_wiring

{
    struct DigitalOutput
    {
        char pin;
        bool is_inverted { false };
    };
}