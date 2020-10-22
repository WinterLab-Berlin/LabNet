#pragma once

#include <LoggingFacility.h>
#include <SFML/Audio.hpp>
#include <map>
#include <memory>
#include <so_5/all.hpp>
#include "Signals.h"

namespace sound
{
    struct init_sound
    {
        const so_5::mbox_t mbox;
    };

    class SignalGenerator : public so_5::agent_t
    {
    public:
        SignalGenerator(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, Logger logger);
        ~SignalGenerator();

    private:
        void so_define_agent() override;
        void so_evt_start() override;
        void so_evt_finish() override;

        bool turn_sound_on(uint32_t id);
        bool turn_sound_off(uint32_t id);

        so_5::state_t init_state { this, "init_state" };
        so_5::state_t running_state { this, "running_state" };
        so_5::state_t paused_state { this, "paused_state" };

        Logger _logger;
        const so_5::mbox_t _events_box;
        const so_5::mbox_t _interfaces_manager_box;
        const so_5::mbox_t _self_box;

        const unsigned SAMPLE_RATE = 44100;
        const unsigned SAMPLES = 44100;
        sf::Sound _sineWaveSound;
        sf::SoundBuffer _soundBuffer;
        sf::Int16* _rawBuffer;

        int32_t _signal_id;
        std::map<uint32_t, std::unique_ptr<Signal>> _signals;
    };
}