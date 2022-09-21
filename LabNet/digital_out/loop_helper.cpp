#include "loop_helper.h"
#include "../interface/digital_messages.h"
#include "../network/protocol_all.h"
#include "loop_messages.h"
#include <climits>

namespace LabNet::digital_out
{
    struct NextTimeStep
    {
    };

    LoopHelper::LoopHelper(context_t ctx, log::Logger logger, so_5::mbox_t dig_out_box, so_5::mbox_t report_box)
        : so_5::agent_t(ctx)
        , logger_(logger)
        , dig_out_box_(dig_out_box)
        , report_box_(report_box)
        , gpio_box_(ctx.environment().create_mbox("gpio"))
        , uart_box_(ctx.environment().create_mbox("uart"))
        , gpio_wiring_box_(ctx.environment().create_mbox("gpioWiring"))
        , uart_board_box_(ctx.environment().create_mbox("uart_board"))
    {
    }

    LoopHelper::~LoopHelper()
    {
    }

    void LoopHelper::so_define_agent()
    {
        this >>= wait_state_;

        wait_state_
            .event([this](mhood_t<LabNetProt::Client::StartDigitalOutLoop> mes) {
                loop_pause_ = mes->loop_pause();

                for (int i = 0; i < mes->digital_outputs_size(); i++)
                {
                    DigitalOutputParameter par;

                    auto interface = mes->digital_outputs()[i].id().interface();
                    if (interface == LabNetProt::INTERFACE_IO_BOARD)
                    {
                        par.id.interface = interface::Interfaces::IoBoard;
                        so_5::send<interface::digital_messages::SetDigitalOut>(gpio_box_, par.id.interface, mes->digital_outputs()[i].id().pin(), false, so_direct_mbox());
                    }
                    else if (interface == LabNetProt::INTERFACE_GPIO_WIRINGPI)
                    {
                        par.id.interface = interface::Interfaces::GpioWiring;
                        so_5::send<interface::digital_messages::SetDigitalOut>(gpio_wiring_box_, par.id.interface, mes->digital_outputs()[i].id().pin(), false, so_direct_mbox());
                    }
                    else if (interface == LabNetProt::INTERFACE_UART1)
                    {
                        par.id.interface = interface::Interfaces::Uart1;
                        so_5::send<interface::digital_messages::SetDigitalOut>(uart_box_, par.id.interface, mes->digital_outputs()[i].id().pin(), false, so_direct_mbox());
                    }
                    else if (interface == LabNetProt::INTERFACE_UART2)
                    {
                        par.id.interface = interface::Interfaces::Uart2;
                        so_5::send<interface::digital_messages::SetDigitalOut>(uart_box_, par.id.interface, mes->digital_outputs()[i].id().pin(), false, so_direct_mbox());
                    }
                    else if (interface == LabNetProt::INTERFACE_UART3)
                    {
                        par.id.interface = interface::Interfaces::Uart3;
                        so_5::send<interface::digital_messages::SetDigitalOut>(uart_box_, par.id.interface, mes->digital_outputs()[i].id().pin(), false, so_direct_mbox());
                    }
                    else if (interface == LabNetProt::INTERFACE_UART4)
                    {
                        par.id.interface = interface::Interfaces::Uart4;
                        so_5::send<interface::digital_messages::SetDigitalOut>(uart_box_, par.id.interface, mes->digital_outputs()[i].id().pin(), false, so_direct_mbox());
                    }
                    else if (interface == LabNetProt::INTERFACE_UART_BOARD)
                    {
                        par.id.interface = interface::Interfaces::UartBoard;
                        so_5::send<interface::digital_messages::SetDigitalOut>(uart_board_box_, par.id.interface, mes->digital_outputs()[i].id().pin(), false, so_direct_mbox());
                    }
                    else
                    {
                        continue;
                    }

                    par.id.pin = mes->digital_outputs()[i].id().pin();
                    par.duration = mes->digital_outputs()[i].duration();
                    par.offset = mes->digital_outputs()[i].offset();
                    loop_.push_back(par);
                }

                start_ = std::chrono::high_resolution_clock::now();
                so_5::send<NextTimeStep>(so_direct_mbox());

                this >>= running_state_;
            })
            .event([this](mhood_t<LabNetProt::Client::StopDigitalOutLoop> mes) {
                so_5::send<LoopStopped>(report_box_, mes->loop_name());
                this >>= stoped_state_;
                so_deregister_agent_coop_normally();
            })
            .event([this](mhood_t<AbortLoopHelper> mes) {
                this >>= stoped_state_;
                so_deregister_agent_coop_normally();
            });

        running_state_
            .event([this](mhood_t<LabNetProt::Client::StopDigitalOutLoop> mes) {
                so_5::send<LoopStopped>(report_box_, mes->loop_name());
                this >>= stoped_state_;

                so_deregister_agent_coop_normally();
            })
            .event([this](mhood_t<AbortLoopHelper> mes) {
                this >>= stoped_state_;
                so_deregister_agent_coop_normally();
            })
            .event([this](mhood_t<PauseLoopHelper> mes) {
                for (auto& digOut : loop_)
                {
                    digOut.handled = false;
                    digOut.state = false;
                }

                this >>= pause_state_;
            })
            .event([this](mhood_t<NextTimeStep> mes) {
                uint wait = UINT_MAX;
                auto now = std::chrono::high_resolution_clock::now();
                auto fsec = now - start_;
                auto d = std::chrono::duration_cast<std::chrono::milliseconds>(fsec);

                for (auto& digOut : loop_)
                {
                    if (!digOut.handled)
                    {
                        wait = 1;

                        if (digOut.state)
                        {
                            uint offTime = digOut.offset + digOut.duration;
                            if (offTime <= d.count())
                            {
                                TurnPinOff(digOut.id);
                                digOut.handled = true;
                            }
                        }
                        else if (digOut.offset <= d.count())
                        {
                            digOut.state = true;
                            TurnPinOn(digOut.id);
                        }
                    }
                }

                if (wait == UINT_MAX)
                {
                    for (auto& digOut : loop_)
                    {
                        digOut.handled = false;
                        digOut.state = false;
                    }

                    start_ = std::chrono::high_resolution_clock::now();
                    start_ += std::chrono::milliseconds(loop_pause_);
                    so_5::send_delayed<NextTimeStep>(so_direct_mbox(), std::chrono::milliseconds(loop_pause_));
                }
                else
                {
                    so_5::send_delayed<NextTimeStep>(so_direct_mbox(), std::chrono::milliseconds(wait));
                }
            });

        pause_state_
            .event([this](mhood_t<AbortLoopHelper> mes) {
                this >>= stoped_state_;
                so_deregister_agent_coop_normally();
            })
            .event([this](mhood_t<ContinueLoopHelper> mes) {
                start_ = std::chrono::high_resolution_clock::now();
                so_5::send<NextTimeStep>(so_direct_mbox());

                this >>= running_state_;
            });
    }

    void LoopHelper::TurnPinOn(PinId& id)
    {
        if (id.interface == interface::Interfaces::IoBoard)
        {
            so_5::send<interface::digital_messages::SetDigitalOut>(gpio_box_, id.interface, id.pin, true, so_direct_mbox());
        }
        else if (id.interface == interface::Interfaces::GpioWiring)
        {
            so_5::send<interface::digital_messages::SetDigitalOut>(gpio_wiring_box_, id.interface, id.pin, true, so_direct_mbox());
        }
        else if (id.interface == interface::Interfaces::Uart0
            || id.interface == interface::Interfaces::Uart1
            || id.interface == interface::Interfaces::Uart2
            || id.interface == interface::Interfaces::Uart3
            || id.interface == interface::Interfaces::Uart4)
        {
            so_5::send<interface::digital_messages::SetDigitalOut>(uart_box_, id.interface, id.pin, true, so_direct_mbox());
        }
        else if (id.interface == interface::Interfaces::UartBoard)
        {
            so_5::send<interface::digital_messages::SetDigitalOut>(uart_board_box_, id.interface, id.pin, true, so_direct_mbox());
        }
    }

    void LoopHelper::TurnPinOff(PinId& id)
    {
        if (id.interface == interface::Interfaces::IoBoard)
        {
            so_5::send<interface::digital_messages::SetDigitalOut>(gpio_box_, id.interface, id.pin, false, so_direct_mbox());
        }
        else if (id.interface == interface::Interfaces::GpioWiring)
        {
            so_5::send<interface::digital_messages::SetDigitalOut>(gpio_wiring_box_, id.interface, id.pin, false, so_direct_mbox());
        }
        else if (id.interface == interface::Interfaces::Uart0
            || id.interface == interface::Interfaces::Uart1
            || id.interface == interface::Interfaces::Uart2
            || id.interface == interface::Interfaces::Uart3
            || id.interface == interface::Interfaces::Uart4)
        {
            so_5::send<interface::digital_messages::SetDigitalOut>(uart_box_, id.interface, id.pin, false, so_direct_mbox());
        }
        else if (id.interface == interface::Interfaces::UartBoard)
        {
            so_5::send<interface::digital_messages::SetDigitalOut>(uart_board_box_, id.interface, id.pin, false, so_direct_mbox());
        }
    }
}