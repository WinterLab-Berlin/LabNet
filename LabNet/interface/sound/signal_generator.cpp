

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

SignalGenerator::SignalGenerator(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, log::Logger logger)
    : so_5::agent_t(ctx)
    , self_box_(self_box)
    , interfaces_manager_box_(interfaces_manager_box)
    , events_box_(events_box)
    , logger_(logger)
    , signals_()
    , sine_wave_sound_()
{
    signal_id_ = -1;

    raw_buffer_ = new sf::Int16[kSamples];
}

SignalGenerator::~SignalGenerator()
{
    sine_wave_sound_.stop();
    delete[] raw_buffer_;
}

void SignalGenerator::so_evt_finish()
{
    so_5::send<InterfaceStopped>(interfaces_manager_box_, Interfaces::Sound);
    logger_->WriteInfoEntry("sound finished");
}

void SignalGenerator::so_evt_start()
{
    logger_->WriteInfoEntry("sound started");
}

void SignalGenerator::so_define_agent()
{
    this >>= init_state_;

    init_state_
        .event(self_box_,
            [this](const mhood_t<init_sound>& mes) {
                so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::Sound, true);

                this >>= running_state_;
            });

    running_state_
        .event(self_box_,
            [this](const mhood_t<init_sound>& msg) {
                so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::Sound, true);
            })
        .event(self_box_,
            [this](std::shared_ptr<LabNetProt::Client::DefineSineTone> msg) {
                uint32_t pin = msg->id();

                signals_[pin] = std::unique_ptr<Signal>(new Signal { SignalType::SineWave, pin, msg->frequenz(), msg->volume() });

                so_5::send<digital_messages::DigitalOutInitResult>(interfaces_manager_box_, Interfaces::Sound, pin, true);
            })
        .event(self_box_,
            [this](const mhood_t<digital_messages::SetDigitalOut>& msg) {
                bool res;
                if (msg->state)
                    TurnSoundOn(msg->pin);
                else
                    TurnSoundOff(msg->pin);
                
                if (res)
                {
                    so_5::send<digital_messages::ReturnDigitalOutState>(msg->mbox, Interfaces::Sound, msg->pin, msg->state, std::chrono::high_resolution_clock::now());
                }
                else
                {
                    so_5::send<digital_messages::InvalidDigitalOutPin>(msg->mbox, Interfaces::Sound, msg->pin);
                }
            })
        .event(self_box_,
            [this](const mhood_t<PauseInterface>& msg) {
                signal_id_ = -1;
                sine_wave_sound_.stop();

                this >>= paused_state_;
            });

    paused_state_
        .event(self_box_,
            [this](const mhood_t<ContinueInterface>& msg) {
                this >>= running_state_;
            });
}

bool SignalGenerator::TurnSoundOn(uint32_t id)
{
    auto it = signals_.find(id);
    if (it == signals_.end())
        return false;

    if (signal_id_ != id)
    {
        
        switch (it->second->type)
        {
            case SignalType::SineWave:
            {
                double sinStep = 2 * M_PI * it->second->frequenz / kSampleRate;
                double sinPos = 0;
                float vol = it->second->volume * SHRT_MAX;
                for (int i = 0; i < kSamples; i++)
                {
                    /* Just fill the stream with sine! */
                    raw_buffer_[i] = (vol * sinf(sinPos));
                    sinPos += sinStep;
                }

                if (!sound_buffer_.loadFromSamples(raw_buffer_, kSamples, 1, kSampleRate))
                {
                    logger_->WriteErrorEntry("sound buffer load error");
                }
                sine_wave_sound_.setBuffer(sound_buffer_);
                sine_wave_sound_.setLoop(true);
                signal_id_ = id;
            }
                break;
            default:
                break;
        }
    }

    sine_wave_sound_.play();

    return true;
}

bool SignalGenerator::TurnSoundOff(uint32_t id)
{
    auto it = signals_.find(id);
    if (it == signals_.end())
        return false;

    if (signal_id_ != id)
    {
        return false;
    }
    else
    {
        sine_wave_sound_.pause();
        return true;
    }
}
