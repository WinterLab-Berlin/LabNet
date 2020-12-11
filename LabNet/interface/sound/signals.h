#pragma once

namespace LabNet::interface::sound
{
    enum class SignalType
    {
        SineWave
    };

    struct Signal
    {
        SignalType type;
        uint32_t id;
        uint32_t frequenz;
        float volume;
    };
}