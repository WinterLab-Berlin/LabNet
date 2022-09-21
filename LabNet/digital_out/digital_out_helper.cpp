#include "digital_out_helper.h"
#include "../interface/digital_messages.h"
#include "../interface/interface_messages.h"
#include "../network/server_messages.h"
#include "../network/protocol_all.h"
#include "../helper/reset_msg.h"
#include "loop_helper.h"
#include "loop_messages.h"
#include "pulse_helper.h"

using namespace LabNet::digital_out;

DigitalOutHelper::DigitalOutHelper(context_t ctx, log::Logger logger)
    : so_5::agent_t(ctx)
    , self_box_(ctx.environment().create_mbox("dig_out_helper"))
    , gpio_box_(ctx.environment().create_mbox("gpio"))
    , uart_box_(ctx.environment().create_mbox("uart"))
    , gpio_wiring_box_(ctx.environment().create_mbox("gpioWiring"))
    , sound_box_(ctx.environment().create_mbox("sound"))
    , server_out_box_(ctx.env().create_mbox("server_out"))
    , server_in_box_(ctx.env().create_mbox("server_in"))
    , uart_board_box_(ctx.env().create_mbox("uart_board"))
    , logger_(logger)
{
}

DigitalOutHelper::~DigitalOutHelper()
{
}

void DigitalOutHelper::so_evt_start()
{
}

void DigitalOutHelper::so_define_agent()
{
    this >>= running_state_;

    running_state_
        .event(server_out_box_,
            [this](const mhood_t<interface::PauseInterface>& msg) {
                for (auto& loop : loop_helper_)
                {
                    so_5::send<PauseLoopHelper>(loop.second);
                }

                for (auto& out : pulse_helper_)
                {
                    so_5::send<StopHelper>(out.second);
                }
                pulse_helper_.clear();

                this >>= paused_state_;
            })
        .event(self_box_,
            [this](const mhood_t<LabNet::helper::ResetRequest>& msg) {
                for (auto& loop : loop_helper_)
                {
                    so_5::send<AbortLoopHelper>(loop.second);
                }
                loop_helper_.clear();

                for (auto& pulse : pulse_helper_)
                {
                    so_5::send<StopHelper>(pulse.second);
                }
                pulse_helper_.clear();

                so_5::send<LabNet::helper::ResetDoneResponse>(msg->response_box, LabNet::helper::ResponseId::DigitalOutHelper);
            })
        .event(server_out_box_,
            [this](std::shared_ptr<LabNetProt::Client::DigitalOutSet> msg) {
                auto interface = msg->id().interface();

                if (interface == LabNetProt::INTERFACE_IO_BOARD)
                {
                    PinId id { interface::Interfaces::IoBoard, msg->id().pin() };

                    if (pulse_helper_.count(id) > 0)
                    {
                        so_5::send<JustSwitch>(pulse_helper_[id], msg->state());
                    }
                    else
                    {
                        so_5::send<interface::digital_messages::SetDigitalOut>(gpio_box_, interface::Interfaces::IoBoard, msg->id().pin(), msg->state(), server_in_box_);
                    }
                }
                else if (interface == LabNetProt::INTERFACE_GPIO_WIRINGPI)
                {
                    PinId id { interface::Interfaces::GpioWiring, msg->id().pin() };

                    if (pulse_helper_.count(id) > 0)
                    {
                        so_5::send<JustSwitch>(pulse_helper_[id], msg->state());
                    }
                    else
                    {
                        so_5::send<interface::digital_messages::SetDigitalOut>(gpio_wiring_box_, interface::Interfaces::GpioWiring, msg->id().pin(), msg->state(), server_in_box_);
                    }
                }
                else if (interface == LabNetProt::INTERFACE_UART0)
                {
                    PinId id { interface::Interfaces::Uart0, msg->id().pin() };

                    if (pulse_helper_.count(id) > 0)
                    {
                        so_5::send<JustSwitch>(pulse_helper_[id], msg->state());
                    }
                    else
                    {
                        so_5::send<interface::digital_messages::SetDigitalOut>(uart_box_, interface::Interfaces::Uart0, msg->id().pin(), msg->state(), server_in_box_);
                    }
                }
                else if (interface == LabNetProt::INTERFACE_UART1)
                {
                    PinId id { interface::Interfaces::Uart1, msg->id().pin() };

                    if (pulse_helper_.count(id) > 0)
                    {
                        so_5::send<JustSwitch>(pulse_helper_[id], msg->state());
                    }
                    else
                    {
                        so_5::send<interface::digital_messages::SetDigitalOut>(uart_box_, interface::Interfaces::Uart1, msg->id().pin(), msg->state(), server_in_box_);
                    }
                }
                else if (interface == LabNetProt::INTERFACE_UART2)
                {
                    PinId id { interface::Interfaces::Uart2, msg->id().pin() };

                    if (pulse_helper_.count(id) > 0)
                    {
                        so_5::send<JustSwitch>(pulse_helper_[id], msg->state());
                    }
                    else
                    {
                        so_5::send<interface::digital_messages::SetDigitalOut>(uart_box_, interface::Interfaces::Uart2, msg->id().pin(), msg->state(), server_in_box_);
                    }
                }
                else if (interface == LabNetProt::INTERFACE_UART3)
                {
                    PinId id { interface::Interfaces::Uart3, msg->id().pin() };

                    if (pulse_helper_.count(id) > 0)
                    {
                        so_5::send<JustSwitch>(pulse_helper_[id], msg->state());
                    }
                    else
                    {
                        so_5::send<interface::digital_messages::SetDigitalOut>(uart_box_, interface::Interfaces::Uart3, msg->id().pin(), msg->state(), server_in_box_);
                    }
                }
                else if (interface == LabNetProt::INTERFACE_UART4)
                {
                    PinId id { interface::Interfaces::Uart4, msg->id().pin() };

                    if (pulse_helper_.count(id) > 0)
                    {
                        so_5::send<JustSwitch>(pulse_helper_[id], msg->state());
                    }
                    else
                    {
                        so_5::send<interface::digital_messages::SetDigitalOut>(uart_box_, interface::Interfaces::Uart4, msg->id().pin(), msg->state(), server_in_box_);
                    }
                }
                else if (interface == LabNetProt::INTERFACE_SOUND)
                {
                    PinId id { interface::Interfaces::Sound, msg->id().pin() };

                    if (pulse_helper_.count(id) > 0)
                    {
                        so_5::send<JustSwitch>(pulse_helper_[id], msg->state());
                    }
                    else
                    {
                        so_5::send<interface::digital_messages::SetDigitalOut>(sound_box_, interface::Interfaces::Sound, msg->id().pin(), msg->state(), server_in_box_);
                    }
                }
                else if (interface == LabNetProt::INTERFACE_UART_BOARD)
                {
                    PinId id { interface::Interfaces::UartBoard, msg->id().pin() };

                    if (pulse_helper_.count(id) > 0)
                    {
                        so_5::send<JustSwitch>(pulse_helper_[id], msg->state());
                    }
                    else
                    {
                        so_5::send<interface::digital_messages::SetDigitalOut>(uart_board_box_, interface::Interfaces::UartBoard, msg->id().pin(), msg->state(), server_in_box_);
                    }
                }
            })
        .event(server_out_box_,
            [this](std::shared_ptr<LabNetProt::Client::DigitalOutPulse> msg) {
                auto interface = msg->id().interface();
                if (interface == LabNetProt::INTERFACE_IO_BOARD)
                {
                    PinId id { interface::Interfaces::IoBoard, msg->id().pin() };

                    if (pulse_helper_.count(id) == 0)
                    {
                        auto coop = so_5::create_child_coop(*this);
                        auto a = coop->make_agent<PulseHelper>(logger_, self_box_, server_in_box_, gpio_box_, id.interface, id.pin);

                        pulse_helper_[id] = a->so_direct_mbox();

                        so_environment().register_coop(std::move(coop));
                    }

                    so_5::send<StartPulse>(pulse_helper_[id], msg->high_duration(), msg->low_duration(), msg->pulses());
                }
                else if (interface == LabNetProt::INTERFACE_GPIO_WIRINGPI)
                {
                    PinId id { interface::Interfaces::GpioWiring, msg->id().pin() };

                    if (pulse_helper_.count(id) == 0)
                    {
                        auto coop = so_5::create_child_coop(*this);
                        auto a = coop->make_agent<PulseHelper>(logger_, self_box_, server_in_box_, gpio_wiring_box_, id.interface, id.pin);

                        pulse_helper_[id] = a->so_direct_mbox();

                        so_environment().register_coop(std::move(coop));
                    }

                    so_5::send<StartPulse>(pulse_helper_[id], msg->high_duration(), msg->low_duration(), msg->pulses());
                }
                else if (interface == LabNetProt::INTERFACE_UART0
                    || interface == LabNetProt::INTERFACE_UART1
                    || interface == LabNetProt::INTERFACE_UART2
                    || interface == LabNetProt::INTERFACE_UART3
                    || interface == LabNetProt::INTERFACE_UART4)
                {
                    PinId id { static_cast<interface::Interfaces>(interface), msg->id().pin() };

                    if (pulse_helper_.count(id) == 0)
                    {
                        auto coop = so_5::create_child_coop(*this);
                        auto a = coop->make_agent<PulseHelper>(logger_, self_box_, server_in_box_, uart_box_, id.interface, id.pin);

                        pulse_helper_[id] = a->so_direct_mbox();

                        so_environment().register_coop(std::move(coop));
                    }

                    so_5::send<StartPulse>(pulse_helper_[id], msg->high_duration(), msg->low_duration(), msg->pulses());
                }
                else if (interface == LabNetProt::INTERFACE_SOUND)
                {
                    PinId id { interface::Interfaces::Sound, msg->id().pin() };

                    if (pulse_helper_.count(id) == 0)
                    {
                        auto coop = so_5::create_child_coop(*this);
                        auto a = coop->make_agent<PulseHelper>(logger_, self_box_, server_in_box_, sound_box_, id.interface, id.pin);

                        pulse_helper_[id] = a->so_direct_mbox();

                        so_environment().register_coop(std::move(coop));
                    }

                    so_5::send<StartPulse>(pulse_helper_[id], msg->high_duration(), msg->low_duration(), msg->pulses());
                }
                else if (interface == LabNetProt::INTERFACE_UART_BOARD)
                {
                    PinId id { interface::Interfaces::UartBoard, msg->id().pin() };

                    if (pulse_helper_.count(id) == 0)
                    {
                        auto coop = so_5::create_child_coop(*this);
                        auto a = coop->make_agent<PulseHelper>(logger_, self_box_, server_in_box_, uart_board_box_, id.interface, id.pin);

                        pulse_helper_[id] = a->so_direct_mbox();

                        so_environment().register_coop(std::move(coop));
                    }

                    so_5::send<StartPulse>(pulse_helper_[id], msg->high_duration(), msg->low_duration(), msg->pulses());
                }
            })
        .event(server_out_box_,
            [this](std::shared_ptr<LabNetProt::Client::StartDigitalOutLoop> msg) {
                std::string loopName = msg->loop_name();

                if (loopName.size() > 0)
                {
                    if (loop_helper_.count(loopName) != 0)
                    {
                        so_5::send<AbortLoopHelper>(loop_helper_[loopName]);
                        loop_helper_.erase(loopName);
                    }

                    if (msg->digital_outputs_size() > 0)
                    {
                        auto coop = so_5::create_child_coop(*this);
                        auto a = coop->make_agent<LoopHelper>(logger_, self_box_, server_in_box_);
                        loop_helper_[loopName] = a->so_direct_mbox();
                        so_environment().register_coop(std::move(coop));

                        so_5::send<LabNetProt::Client::StartDigitalOutLoop>(loop_helper_[loopName], *msg);

                        std::shared_ptr<LabNetProt::Server::DigitalOutLoopStartResult> initRes = std::make_shared<LabNetProt::Server::DigitalOutLoopStartResult>();
                        initRes->set_loop_name(msg->loop_name());
                        initRes->set_is_succeed(true);
                        so_5::send<std::shared_ptr<LabNetProt::Server::DigitalOutLoopStartResult>>(server_in_box_, initRes);
                    }
                    else
                    {
                        std::shared_ptr<LabNetProt::Server::DigitalOutLoopStartResult> initRes = std::make_shared<LabNetProt::Server::DigitalOutLoopStartResult>();
                        initRes->set_loop_name(msg->loop_name());
                        initRes->set_is_succeed(false);
                        so_5::send<std::shared_ptr<LabNetProt::Server::DigitalOutLoopStartResult>>(server_in_box_, initRes);
                    }
                }
            })
        .event(server_out_box_,
            [this](std::shared_ptr<LabNetProt::Client::StopDigitalOutLoop> msg) {
                auto loopName = msg->loop_name();

                if (loop_helper_.count(loopName) == 0)
                {
                }
                else
                {
                    so_5::send<LabNetProt::Client::StopDigitalOutLoop>(loop_helper_[loopName], *msg);
                    loop_helper_.erase(loopName);
                }
            });

    paused_state_
        .event(server_out_box_,
            [this](const mhood_t<interface::ContinueInterface>& msg) {
                this >>= running_state_;

                for (auto& loop : loop_helper_)
                {
                    so_5::send<ContinueLoopHelper>(loop.second);
                }
            });
}