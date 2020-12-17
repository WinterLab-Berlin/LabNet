#pragma once

#include <logging_facility.h>
#include <SFML/Audio.hpp>
#include <map>
#include <vector>
#include <memory>
#include <so_5/all.hpp>
#include "signals.h"

namespace LabNet::interface::sound
{
    struct InitSound
    {
        const so_5::mbox_t mbox;
    };

    class SignalGenerator : public so_5::agent_t
    {
    public:
        SignalGenerator(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, log::Logger logger);
        ~SignalGenerator();

    private:
        void so_define_agent() override;
        void so_evt_start() override;
        void so_evt_finish() override;

        bool TurnSoundOn(uint32_t id);
        bool TurnSoundOff(uint32_t id);
        bool InitNewSignal(std::shared_ptr<Signal> signal);

        so_5::state_t init_state_ { this, "init_state" };
        so_5::state_t running_state_ { this, "running_state" };
        so_5::state_t paused_state_ { this, "paused_state" };

        log::Logger logger_;
        const so_5::mbox_t events_box_;
        const so_5::mbox_t interfaces_manager_box_;
        const so_5::mbox_t self_box_;

        const uint32_t kSampleRate = 192000;
        sf::Sound sine_wave_sound_;
        sf::SoundBuffer sound_buffer_;

        int32_t signal_id_;
        std::map<uint32_t, std::shared_ptr<Signal>> signals_;
    };
}