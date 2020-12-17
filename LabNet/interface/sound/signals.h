#pragma once

#include <vector>
#include <SFML/Audio.hpp>

namespace LabNet::interface::sound
{
    enum class SignalType
    {
        SineWave = 0,
        SquareWave = 1,
        TriangleWave = 2,
        SawToothWave = 3,
        WhiteNoise = 4,
        PinkNoise = 5,
        Sweep = 6
    };

    struct Signal
    {
        SignalType type;
        uint32_t id;
        uint32_t frequency = 3;
        uint32_t frequency_end = 4;
        float sweep_length_seconds = 5;
        float volume = 6;
        std::unique_ptr<std::vector<sf::Int16>> buffer;
    };
}