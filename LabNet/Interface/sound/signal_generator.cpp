

#define _USE_MATH_DEFINES
#include <math.h>

#include <climits>
#include <cmath>

#include "../../network/LabNet.pb.h"
#include "../../network/LabNetClient.pb.h"
#include "../digital_messages.h"
#include "../interface_messages.h"
#include "signal_generator.h"

using namespace LabNet::interface::sound;

SignalGenerator::SignalGenerator(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, Logger logger)
    : so_5::agent_t(ctx)
    , _self_box(self_box)
    , _interfaces_manager_box(interfaces_manager_box)
    , _events_box(events_box)
    , _logger(logger)
    , _signals()
    , _sine_wave_sound()
{
    _signal_id = -1;

    _raw_buffer = new sf::Int16[kSamples];
}

SignalGenerator::~SignalGenerator()
{
    _sine_wave_sound.stop();
    delete[] _raw_buffer;
}

void SignalGenerator::so_evt_finish()
{
    so_5::send<InterfaceStopped>(_interfaces_manager_box, Interfaces::SOUND);
    _logger->writeInfoEntry("sound finished");
}

void SignalGenerator::so_evt_start()
{
    _logger->writeInfoEntry("sound started");
}

void SignalGenerator::so_define_agent()
{
    this >>= init_state;

    init_state
        .event(_self_box,
            [this](const mhood_t<init_sound>& mes) {
                so_5::send<InterfaceInitResult>(_interfaces_manager_box, Interfaces::SOUND, true);

                this >>= running_state;
            });

    running_state
        .event(_self_box,
            [this](const mhood_t<init_sound>& msg) {
                so_5::send<InterfaceInitResult>(_interfaces_manager_box, Interfaces::SOUND, true);
            })
        .event(_self_box,
            [this](std::shared_ptr<LabNetProt::Client::DefineSineTone> msg) {
                uint32_t pin = msg->id();

                _signals[pin] = std::unique_ptr<Signal>(new Signal { SignalType::SineWave, pin, msg->frequenz(), msg->volume() });

                so_5::send<digital_messages::DigitalOutInitResult>(_interfaces_manager_box, Interfaces::SOUND, pin, true);
            })
        .event(_self_box,
            [this](const mhood_t<digital_messages::SetDigitalOut>& msg) {
                bool res;
                if (msg->state)
                    TurnSoundOn(msg->pin);
                else
                    TurnSoundOff(msg->pin);
                
                if (res)
                {
                    so_5::send<digital_messages::ReturnDigitalOutState>(msg->mbox, Interfaces::IO_BOARD, msg->pin, msg->state, std::chrono::high_resolution_clock::now());
                }
                else
                {
                    so_5::send<digital_messages::InvalidDigitalOutPin>(msg->mbox, Interfaces::SOUND, msg->pin);
                }
            })
        .event(_self_box,
            [this](const mhood_t<PauseInterface>& msg) {
                _signal_id = -1;
                _sine_wave_sound.stop();

                this >>= paused_state;
            });

    paused_state
        .event(_self_box,
            [this](const mhood_t<ContinueInterface>& msg) {
                this >>= running_state;
            });
}

bool SignalGenerator::TurnSoundOn(uint32_t id)
{
    auto it = _signals.find(id);
    if (it == _signals.end())
        return false;

    if (_signal_id != id)
    {
        
        switch (it->second->type)
        {
            case SignalType::SineWave:
            {
                double sinStep = 2 * M_PI * it->second->frequenz / kSample_rate;
                double sinPos = 0;
                float vol = it->second->volume * SHRT_MAX;
                for (int i = 0; i < kSamples; i++)
                {
                    /* Just fill the stream with sine! */
                    _raw_buffer[i] = (vol * sinf(sinPos));
                    sinPos += sinStep;
                }

                if (!_sound_buffer.loadFromSamples(_raw_buffer, kSamples, 1, kSample_rate))
                {
                    _logger->writeErrorEntry("sound buffer load error");
                }
                _sine_wave_sound.setBuffer(_sound_buffer);
                _sine_wave_sound.setLoop(true);
                _signal_id = id;
            }
                break;
            default:
                break;
        }
    }

    _sine_wave_sound.play();

    return true;
}

bool SignalGenerator::TurnSoundOff(uint32_t id)
{
    auto it = _signals.find(id);
    if (it == _signals.end())
        return false;

    if (_signal_id != id)
    {
        return false;
    }
    else
    {
        _sine_wave_sound.pause();
        return true;
    }
}
