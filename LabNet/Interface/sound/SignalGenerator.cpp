

#define _USE_MATH_DEFINES
#include <math.h>

#include <climits>
#include <cmath>

#include "../../Network/LabNet.pb.h"
#include "../../Network/LabNetClient.pb.h"
#include "../DigitalMessages.h"
#include "../InterfaceMessages.h"
#include "SignalGenerator.h"

using namespace sound;

SignalGenerator::SignalGenerator(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, Logger logger)
    : so_5::agent_t(ctx)
    , _self_box(self_box)
    , _interfaces_manager_box(interfaces_manager_box)
    , _events_box(events_box)
    , _logger(logger)
    , _signals()
    , _sineWaveSound()
{
    _signal_id = -1;

    _rawBuffer = new sf::Int16[SAMPLES];
}

SignalGenerator::~SignalGenerator()
{
    _sineWaveSound.stop();
    delete[] _rawBuffer;
}

void SignalGenerator::so_evt_finish()
{
    so_5::send<Interface::interface_stopped>(_interfaces_manager_box, Interface::Interfaces::SOUND);
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
                so_5::send<Interface::interface_init_result>(_interfaces_manager_box, Interface::Interfaces::SOUND, true);

                this >>= running_state;
            });

    running_state
        .event(_self_box,
            [this](const mhood_t<init_sound>& msg) {
                so_5::send<Interface::interface_init_result>(_interfaces_manager_box, Interface::Interfaces::SOUND, true);
            })
        .event(_self_box,
            [this](std::shared_ptr<LabNetProt::Client::DefineSineTone> msg) {
                uint32_t pin = msg->id();

                _signals[pin] = std::unique_ptr<Signal>(new Signal { SignalType::SineWave, pin, msg->frequenz(), msg->volume() });

                so_5::send<DigitalMessages::digital_out_init_result>(_interfaces_manager_box, Interface::SOUND, pin, true);
            })
        .event(_self_box,
            [this](const mhood_t<DigitalMessages::set_digital_out>& msg) {
                bool res;
                if (msg->state)
                    turn_sound_on(msg->pin);
                else
                    turn_sound_off(msg->pin);
                
                if (res)
                {
                    so_5::send<DigitalMessages::return_digital_out_state>(msg->mbox, Interface::IO_BOARD, msg->pin, msg->state, std::chrono::high_resolution_clock::now());
                }
                else
                {
                    so_5::send<DigitalMessages::invalid_digital_out_pin>(msg->mbox, Interface::SOUND, msg->pin);
                }
            })
        .event(_self_box,
            [this](const mhood_t<Interface::pause_interface>& msg) {
                _signal_id = -1;
                _sineWaveSound.stop();

                this >>= paused_state;
            });

    paused_state
        .event(_self_box,
            [this](const mhood_t<Interface::continue_interface>& msg) {
                this >>= running_state;
            });
}

bool SignalGenerator::turn_sound_on(uint32_t id)
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
                double sinStep = 2 * M_PI * it->second->frequenz / SAMPLE_RATE;
                double sinPos = 0;
                float vol = it->second->volume * SHRT_MAX;
                for (int i = 0; i < SAMPLES; i++)
                {
                    /* Just fill the stream with sine! */
                    _rawBuffer[i] = (vol * sinf(sinPos));
                    sinPos += sinStep;
                }

                if (!_soundBuffer.loadFromSamples(_rawBuffer, SAMPLES, 1, SAMPLE_RATE))
                {
                    _logger->writeErrorEntry("sound buffer load error");
                }
                _sineWaveSound.setBuffer(_soundBuffer);
                _sineWaveSound.setLoop(true);
                _signal_id = id;
            }
                break;
            default:
                break;
        }
    }

    _sineWaveSound.play();

    return true;
}

bool SignalGenerator::turn_sound_off(uint32_t id)
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
        _sineWaveSound.pause();
        return true;
    }
}
