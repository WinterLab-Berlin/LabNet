#include "LoopHelper.h"
#include "../Interface/DigitalMessages.h"
#include "../network/ProtocolAll.h"
#include "LoopMessages.h"
#include <climits>

struct next_time_step
{
};

DigitalOut::LoopHelper::LoopHelper(context_t ctx, Logger logger, so_5::mbox_t dig_out_box, so_5::mbox_t lab_net_box)
    : so_5::agent_t(ctx)
    , _logger(logger)
    , _digOutBox(dig_out_box)
    , _labNetBox(lab_net_box)
    , _gpioBox(ctx.environment().create_mbox("gpio"))
    , _uartBox(ctx.environment().create_mbox("uart"))
    , _gpioWiringBox(ctx.environment().create_mbox("gpioWiring"))
{
}

DigitalOut::LoopHelper::~LoopHelper()
{
}

void DigitalOut::LoopHelper::so_define_agent()
{
    this >>= _waitState;

    _waitState
        .event([this](mhood_t<LabNetProt::Client::StartDigitalOutLoop> mes) {
            _loopPause = mes->loop_pause();

            for (int i = 0; i < mes->digital_outputs_size(); i++)
            {
                DigitalOutputParameter par;

                auto interface = mes->digital_outputs()[i].id().interface();
                if (interface == LabNetProt::INTERFACE_IO_BOARD)
                {
                    par.id.interface = Interface::IO_BOARD;
                    so_5::send<DigitalMessages::set_digital_out>(_gpioBox, par.id.interface, mes->digital_outputs()[i].id().pin(), false, so_direct_mbox());
                }
                else if (interface == LabNetProt::INTERFACE_GPIO_WIRINGPI)
                {
                    par.id.interface = Interface::GPIO_WIRING;
                    so_5::send<DigitalMessages::set_digital_out>(_gpioWiringBox, par.id.interface, mes->digital_outputs()[i].id().pin(), false, so_direct_mbox());
                }
                else if (interface == LabNetProt::INTERFACE_UART1)
                {
                    par.id.interface = Interface::UART1;
                    so_5::send<DigitalMessages::set_digital_out>(_uartBox, par.id.interface, mes->digital_outputs()[i].id().pin(), false, so_direct_mbox());
                }
                else if (interface == LabNetProt::INTERFACE_UART2)
                {
                    par.id.interface = Interface::UART2;
                    so_5::send<DigitalMessages::set_digital_out>(_uartBox, par.id.interface, mes->digital_outputs()[i].id().pin(), false, so_direct_mbox());
                }
                else if (interface == LabNetProt::INTERFACE_UART3)
                {
                    par.id.interface = Interface::UART3;
                    so_5::send<DigitalMessages::set_digital_out>(_uartBox, par.id.interface, mes->digital_outputs()[i].id().pin(), false, so_direct_mbox());
                }
                else if (interface == LabNetProt::INTERFACE_UART4)
                {
                    par.id.interface = Interface::UART4;
                    so_5::send<DigitalMessages::set_digital_out>(_uartBox, par.id.interface, mes->digital_outputs()[i].id().pin(), false, so_direct_mbox());
                }
                else
                {
                    continue;
                }

                par.id.pin = mes->digital_outputs()[i].id().pin();
                par.duration = mes->digital_outputs()[i].duration();
                par.offset = mes->digital_outputs()[i].offset();
                _loop.push_back(par);
            }

            _start = std::chrono::high_resolution_clock::now();
            so_5::send<next_time_step>(so_direct_mbox());

            this >>= _runningState;
        })
        .event([this](mhood_t<LabNetProt::Client::StopDigitalOutLoop> mes) {
            so_5::send<loop_stopped>(_labNetBox, mes->loop_name());

            so_deregister_agent_coop_normally();
        })
        .event([this](mhood_t<abort_loop_helper> mes) {
            so_deregister_agent_coop_normally();
        });

    _runningState
        .event([this](mhood_t<LabNetProt::Client::StopDigitalOutLoop> mes) {
            so_5::send<loop_stopped>(_labNetBox, mes->loop_name());

            so_deregister_agent_coop_normally();
        })
        .event([this](mhood_t<abort_loop_helper> mes) {
            so_deregister_agent_coop_normally();
        })
        .event([this](mhood_t<pause_loop_helper> mes) {
            for (auto& digOut : _loop)
            {
                digOut.handled = false;
                digOut.state = false;
            }

            this >>= _pauseState;
        })
        .event([this](mhood_t<next_time_step> mes) {
            uint wait = UINT_MAX;
            auto now = std::chrono::high_resolution_clock::now();
            auto fsec = now - _start;
            auto d = std::chrono::duration_cast<std::chrono::milliseconds>(fsec);

            for (auto& digOut : _loop)
            {
                if (!digOut.handled)
                {
                    wait = 1;

                    if (digOut.state)
                    {
                        uint offTime = digOut.offset + digOut.duration;
                        if (offTime <= d.count())
                        {
                            turn_pin_off(digOut.id);
                            digOut.handled = true;
                        }
                    }
                    else if (digOut.offset <= d.count())
                    {
                        digOut.state = true;
                        turn_pin_on(digOut.id);
                    }
                }
            }

            if (wait == UINT_MAX)
            {
                for (auto& digOut : _loop)
                {
                    digOut.handled = false;
                    digOut.state = false;
                }

                _start = std::chrono::high_resolution_clock::now();
                _start += std::chrono::milliseconds(_loopPause);
                so_5::send_delayed<next_time_step>(so_direct_mbox(), std::chrono::milliseconds(_loopPause));
            }
            else
            {
                so_5::send_delayed<next_time_step>(so_direct_mbox(), std::chrono::milliseconds(wait));
            }
        });

    _pauseState
        .event([this](mhood_t<abort_loop_helper> mes) {
            so_deregister_agent_coop_normally();
        })
        .event([this](mhood_t<continue_loop_helper> mes) {
            _start = std::chrono::high_resolution_clock::now();
            so_5::send<next_time_step>(so_direct_mbox());

            this >>= _runningState;
        });
}

void DigitalOut::LoopHelper::turn_pin_on(PinId& id)
{
    if (id.interface == Interface::Interfaces::IO_BOARD)
    {
        so_5::send<DigitalMessages::set_digital_out>(_gpioBox, id.interface, id.pin, true, so_direct_mbox());
    }
    else if (id.interface == Interface::Interfaces::GPIO_WIRING)
    {
        so_5::send<DigitalMessages::set_digital_out>(_gpioWiringBox, id.interface, id.pin, true, so_direct_mbox());
    }
    else if (id.interface == Interface::Interfaces::UART0
        || id.interface == Interface::Interfaces::UART1
        || id.interface == Interface::Interfaces::UART2
        || id.interface == Interface::Interfaces::UART3
        || id.interface == Interface::Interfaces::UART4)
    {
        so_5::send<DigitalMessages::set_digital_out>(_uartBox, id.interface, id.pin, true, so_direct_mbox());
    }
}

void DigitalOut::LoopHelper::turn_pin_off(PinId& id)
{
    if (id.interface == Interface::Interfaces::IO_BOARD)
    {
        so_5::send<DigitalMessages::set_digital_out>(_gpioBox, id.interface, id.pin, false, so_direct_mbox());
    }
    else if (id.interface == Interface::Interfaces::GPIO_WIRING)
    {
        so_5::send<DigitalMessages::set_digital_out>(_gpioWiringBox, id.interface, id.pin, false, so_direct_mbox());
    }
    else if (id.interface == Interface::Interfaces::UART0
        || id.interface == Interface::Interfaces::UART1
        || id.interface == Interface::Interfaces::UART2
        || id.interface == Interface::Interfaces::UART3
        || id.interface == Interface::Interfaces::UART4)
    {
        so_5::send<DigitalMessages::set_digital_out>(_uartBox, id.interface, id.pin, false, so_direct_mbox());
    }
}