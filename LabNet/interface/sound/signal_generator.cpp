

#define _USE_MATH_DEFINES
#include <climits>
#include <cmath>
#include <math.h>
#include <random>

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
}

SignalGenerator::~SignalGenerator()
{
    sine_wave_sound_.stop();
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
            [this](const mhood_t<InitSound>& mes) {
                so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::Sound, true);

                this >>= running_state_;
            });

    running_state_
        .event(self_box_,
            [this](const mhood_t<InitSound>& msg) {
                so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::Sound, true);
            })
        .event(self_box_,
            [this](std::shared_ptr<LabNetProt::Client::InitSoundSignal> msg) {
                std::shared_ptr<Signal> signal = std::shared_ptr<Signal>(
                    new Signal { static_cast<SignalType>(msg->signal_type()), msg->id(), msg->frequency(), msg->frequency_end(), msg->sweep_length_seconds(), msg->volume(), std::make_unique<std::vector<sf::Int16>>() });

                bool res = InitNewSignal(signal);
                if (res)
                {
                    signals_[msg->id()] = signal;
                    so_5::send<digital_messages::DigitalOutInitResult>(interfaces_manager_box_, Interfaces::Sound, msg->id(), true);
                }
                else
                {
                    so_5::send<digital_messages::DigitalOutInitResult>(interfaces_manager_box_, Interfaces::Sound, msg->id(), false);
                }
            })
        .event(self_box_,
            [this](const mhood_t<digital_messages::SetDigitalOut>& msg) {
                bool res;
                if (msg->state)
                    res = TurnSoundOn(msg->pin);
                else
                    res = TurnSoundOff(msg->pin);

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

bool SignalGenerator::InitNewSignal(std::shared_ptr<Signal> signal)
{
    bool init_success = true;

    switch (signal->type)
    {
        case SignalType::SineWave:
        {
            double sin_step = 2 * M_PI * static_cast<double>(signal->frequency) / kSampleRate;
            int32_t samples = 10 * (2 * M_PI / sin_step);

            double sin_pos = 0;
            double vol = signal->volume * SHRT_MAX;
            for (int32_t i = 0; i < samples; i++)
            {
                /* Just fill the stream with sine! */
                signal->buffer->push_back(static_cast<sf::Int16>((vol * sinf(sin_pos))));
                sin_pos += sin_step;
            }
        }
        break;
        case SignalType::SquareWave:
        {
            double step = 2 * static_cast<double>(signal->frequency) / kSampleRate;
            int32_t samples = 10 * (2.0 / step);

            double sample_saw = 0;
            double vol = signal->volume * SHRT_MAX;

            for (int32_t i = 0; i < samples; i++)
            {
                sample_saw = fmod(i * step, 2) - 1;
                if (sample_saw >= 0)
                    signal->buffer->push_back(static_cast<sf::Int16>(vol));
                else
                    signal->buffer->push_back(static_cast<sf::Int16>(-vol));
            }
        }
        break;
        case SignalType::TriangleWave:
        {
            double step = 2 * static_cast<double>(signal->frequency) / kSampleRate;
            int32_t samples = 10 * (2.0 / step);

            double sample_saw = 0;
            double vol = signal->volume * SHRT_MAX;

            for (int32_t i = 0; i < samples; i++)
            {
                sample_saw = 2 * fmod(i * step, 2);
                if (sample_saw > 1)
                    sample_saw = 2 - sample_saw;
                if (sample_saw < -1)
                    sample_saw = -2 - sample_saw;

                signal->buffer->push_back(static_cast<sf::Int16>(sample_saw * vol));
            }

            break;
        }
        case SignalType::SawToothWave:
        {
            double step = 2 * static_cast<double>(signal->frequency) / kSampleRate;
            int32_t samples = 10 * (2 / step);

            double sample_saw = 0;
            double vol = signal->volume * SHRT_MAX;

            for (int32_t i = 0; i < samples; i++)
            {
                sample_saw = fmod(i * step, 2) - 1;
                signal->buffer->push_back(static_cast<sf::Int16>(sample_saw * vol));
            }

            break;
        }
        case SignalType::WhiteNoise:
        {
            double vol = signal->volume * SHRT_MAX;
            double sample = 0;

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution dis(-1.0, 1.0);

            for (size_t i = 0; i < kSampleRate; i++)
            {
                sample = dis(gen) * vol;
                signal->buffer->push_back(static_cast<sf::Int16>(sample * vol));
            }

            break;
        }
        case SignalType::PinkNoise:
        {
            double vol = signal->volume * SHRT_MAX;
            double white = 0;
            double pinkNoiseBuffer[7] = { 0 };
            double pink = 0;

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution dis(-1.0, 1.0);

            for (size_t i = 0; i < kSampleRate; i++)
            {
                white = dis(gen);

                pinkNoiseBuffer[0] = 0.99886 * pinkNoiseBuffer[0] + white * 0.0555179;
                pinkNoiseBuffer[1] = 0.99332 * pinkNoiseBuffer[1] + white * 0.0750759;
                pinkNoiseBuffer[2] = 0.96900 * pinkNoiseBuffer[2] + white * 0.1538520;
                pinkNoiseBuffer[3] = 0.86650 * pinkNoiseBuffer[3] + white * 0.3104856;
                pinkNoiseBuffer[4] = 0.55000 * pinkNoiseBuffer[4] + white * 0.5329522;
                pinkNoiseBuffer[5] = -0.7616 * pinkNoiseBuffer[5] - white * 0.0168980;
                pink = pinkNoiseBuffer[0] + pinkNoiseBuffer[1] + pinkNoiseBuffer[2] + pinkNoiseBuffer[3] + pinkNoiseBuffer[4] + pinkNoiseBuffer[5] + pinkNoiseBuffer[6] + white * 0.5362;
                pinkNoiseBuffer[6] = white * 0.115926;

                signal->buffer->push_back(static_cast<sf::Int16>((pink / 8) * vol));
            }

            break;
        }
        case SignalType::Sweep:
        {
            if (signal->sweep_length_seconds <= 0)
            {
                init_success = false;
                break;
            }

            int32_t samples = signal->sweep_length_seconds * kSampleRate;

            double vol = signal->volume * SHRT_MAX;
            double frequency_log = ::log(signal->frequency);
            double frequency_end_log = ::log(signal->frequency_end);
            double multiple = 0;
            double phi = 0;
            double sample = 0;
            double two_pi = 2 * M_PI;

            for (int32_t i = 0; i < samples; i++)
            {
                double f = exp(frequency_log + (i * (frequency_end_log - frequency_log)) / (signal->sweep_length_seconds * kSampleRate));
                multiple = two_pi * f / kSampleRate;
                phi += multiple;
                sample = sin(phi);
                signal->buffer->push_back(static_cast<sf::Int16>(sample * vol));
            }

            break;
        }
        default:
            init_success = false;
            break;
    }

    return init_success;
}

bool SignalGenerator::TurnSoundOn(uint32_t id)
{
    auto it = signals_.find(id);
    if (it == signals_.end())
        return false;

    std::vector<sf::Int16> v;

    if (signal_id_ != id)
    {
        sf::SoundSource::Status status = sine_wave_sound_.getStatus();
        if (status == sf::SoundSource::Status::Playing)
            sine_wave_sound_.stop();

        if (!sound_buffer_.loadFromSamples(it->second->buffer->data(), it->second->buffer->size(), 1, kSampleRate))
        {
            logger_->WriteErrorEntry("sound buffer load error");
        }
        sine_wave_sound_.setBuffer(sound_buffer_);
        sine_wave_sound_.setLoop(true);
        signal_id_ = id;
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
