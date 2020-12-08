#include "pulse_helper.h"
#include "../Interface/digital_messages.h"

namespace LabNet::digital_out
{

    struct TurnOn final : public so_5::signal_t
    {
    };
    struct TurnOff final : public so_5::signal_t
    {
    };

    PulseHelper::PulseHelper(context_t ctx,
        Logger logger,
        so_5::mbox_t dig_out_box,
        so_5::mbox_t lab_net_box,
        so_5::mbox_t interface_box,
        interface::Interfaces interface,
        char pin)
        : so_5::agent_t(ctx)
        , _logger(logger)
        , _digOutBox(dig_out_box)
        , _labNetBox(lab_net_box)
        , _interfaceBox(interface_box)
        , _interface(interface)
        , _pin(pin)
    {
    }

    void PulseHelper::so_define_agent()
    {
        this >>= _waitState;

        _waitState
            .event([this](mhood_t<StartPulse> mes) {
                _highDuration = mes->high_duration;
                _lowDuration = mes->low_duration;
                _pulses = mes->pulses;

                so_5::send<interface::digital_messages::SetDigitalOut>(_interfaceBox, _interface, _pin, true, so_direct_mbox());
                this >>= _startingState;
            })
            .event([this](mhood_t<StopHelper> mes) {
                so_5::send<interface::digital_messages::SetDigitalOut>(_interfaceBox, _interface, _pin, false, _labNetBox);
                so_deregister_agent_coop_normally();
            })
            .event([this](mhood_t<JustSwitch> mes) {
                so_5::send<interface::digital_messages::SetDigitalOut>(_interfaceBox, _interface, _pin, mes->state, _labNetBox);
            });

        _startingState
            .event([this](mhood_t<interface::digital_messages::ReturnDigitalOutState> mes) {
                this >>= _runningState;

                so_5::send<interface::digital_messages::ReturnDigitalOutState>(_labNetBox, _interface, mes->pin, mes->state, std::chrono::high_resolution_clock::now());

                _turnOffTimer = so_5::send_periodic<TurnOff>(
                    so_direct_mbox(),
                    std::chrono::milliseconds(_highDuration),
                    std::chrono::milliseconds(_highDuration + _lowDuration));

                _turnOnTimer = so_5::send_periodic<TurnOn>(
                    so_direct_mbox(),
                    std::chrono::milliseconds(_highDuration + _lowDuration),
                    std::chrono::milliseconds(_highDuration + _lowDuration));
            })
            .event([this](mhood_t<interface::digital_messages::InvalidDigitalOutPin> mes) {
                so_5::send<interface::digital_messages::ReturnDigitalOutState>(_labNetBox, _interface, mes->pin, false, std::chrono::high_resolution_clock::now());
            })
            .event([this](mhood_t<StopHelper> mes) {
                so_5::send<interface::digital_messages::SetDigitalOut>(_interfaceBox, _interface, _pin, false, _labNetBox);
                so_deregister_agent_coop_normally();
            })
            .event([this](mhood_t<JustSwitch> mes) {
                so_5::send<interface::digital_messages::SetDigitalOut>(_interfaceBox, _interface, _pin, mes->state, _labNetBox);
                this >>= _waitState;
            });

        _runningState
            .event([this](mhood_t<TurnOff> mes) {
                if (_pulses < 255)
                    _pulses--;

                if (_pulses <= 0)
                {
                    so_5::send<interface::digital_messages::SetDigitalOut>(_interfaceBox, _interface, _pin, false, _labNetBox);

                    _turnOffTimer.release();
                    _turnOnTimer.release();
                }
                else
                {
                    so_5::send<interface::digital_messages::SetDigitalOut>(_interfaceBox, _interface, _pin, false, so_direct_mbox());
                }
            })
            .event([this](mhood_t<TurnOn> mes) {
                so_5::send<interface::digital_messages::SetDigitalOut>(_interfaceBox, _interface, _pin, true, so_direct_mbox());
            })
            .event([this](mhood_t<StopHelper> mes) {
                so_5::send<interface::digital_messages::SetDigitalOut>(_interfaceBox, _interface, _pin, false, _labNetBox);
                _turnOffTimer.release();
                _turnOnTimer.release();
                so_deregister_agent_coop_normally();
            })
            .event([this](mhood_t<StartPulse> mes) {
                _turnOffTimer.release();
                _turnOnTimer.release();

                _highDuration = mes->high_duration;
                _lowDuration = mes->low_duration;
                _pulses = mes->pulses;

                so_5::send<interface::digital_messages::SetDigitalOut>(_interfaceBox, _interface, _pin, true, so_direct_mbox());
                this >>= _startingState;
            })
            .event([this](mhood_t<JustSwitch> mes) {
                so_5::send<interface::digital_messages::SetDigitalOut>(_interfaceBox, _interface, _pin, mes->state, _labNetBox);
                _turnOffTimer.release();
                _turnOnTimer.release();

                this >>= _waitState;
            });
        ;
    }
}