#include "digital_out_helper.h"
#include "../Interface/digital_messages.h"
#include "../Interface/interface_messages.h"
#include "../network/server_messages.h"
#include "../network/ProtocolAll.h"
#include "../helper/reset_msg.h"
#include "loop_helper.h"
#include "loop_messages.h"
#include "pulse_helper.h"

using namespace LabNet::digital_out;

DigitalOutHelper::DigitalOutHelper(context_t ctx, Logger logger)
    : so_5::agent_t(ctx)
    , _selfBox(ctx.environment().create_mbox("dig_out_helper"))
    , _gpioBox(ctx.environment().create_mbox("gpio"))
    , _uartBox(ctx.environment().create_mbox("uart"))
    , _gpioWiringBox(ctx.environment().create_mbox("gpioWiring"))
    , _soundBox(ctx.environment().create_mbox("sound"))
    , _server_out_box(ctx.env().create_mbox("server_out"))
    , _server_in_box(ctx.env().create_mbox("server_in"))
    , _logger(logger)
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
    this >>= _runningState;

    _runningState
        .event(_server_out_box,
            [this](const mhood_t<interface::PauseInterface>& msg) {
                for (auto& loop : _loopHelper)
                {
                    so_5::send<PauseLoopHelper>(loop.second);
                }

                for (auto& out : _pulseHelper)
                {
                    so_5::send<StopHelper>(out.second);
                }
                _pulseHelper.clear();

                this >>= _pausedState;
            })
        .event(_selfBox,
            [this](const mhood_t<LabNet::helper::ResetRequest>& msg) {
                for (auto& loop : _loopHelper)
                {
                    so_5::send<AbortLoopHelper>(loop.second);
                }
                _loopHelper.clear();

                for (auto& pulse : _pulseHelper)
                {
                    so_5::send<StopHelper>(pulse.second);
                }
                _pulseHelper.clear();

                so_5::send<LabNet::helper::ResetDoneResponse>(msg->response_box, LabNet::helper::ResponseId::DigitalOutHelper);
            })
        .event(_server_out_box,
            [this](std::shared_ptr<LabNetProt::Client::DigitalOutSet> msg) {
                auto interface = msg->id().interface();

                if (interface == LabNetProt::INTERFACE_IO_BOARD)
                {
                    PinId id { interface::Interfaces::IO_BOARD, msg->id().pin() };

                    if (_pulseHelper.count(id) > 0)
                    {
                        so_5::send<JustSwitch>(_pulseHelper[id], msg->state());
                    }
                    else
                    {
                        so_5::send<interface::digital_messages::SetDigitalOut>(_gpioBox, interface::Interfaces::IO_BOARD, msg->id().pin(), msg->state(), _server_in_box);
                    }
                }
                else if (interface == LabNetProt::INTERFACE_GPIO_WIRINGPI)
                {
                    PinId id { interface::Interfaces::GPIO_WIRING, msg->id().pin() };

                    if (_pulseHelper.count(id) > 0)
                    {
                        so_5::send<JustSwitch>(_pulseHelper[id], msg->state());
                    }
                    else
                    {
                        so_5::send<interface::digital_messages::SetDigitalOut>(_gpioWiringBox, interface::Interfaces::GPIO_WIRING, msg->id().pin(), msg->state(), _server_in_box);
                    }
                }
                else if (interface == LabNetProt::INTERFACE_UART0)
                {
                    PinId id { interface::Interfaces::UART0, msg->id().pin() };

                    if (_pulseHelper.count(id) > 0)
                    {
                        so_5::send<JustSwitch>(_pulseHelper[id], msg->state());
                    }
                    else
                    {
                        so_5::send<interface::digital_messages::SetDigitalOut>(_uartBox, interface::Interfaces::UART0, msg->id().pin(), msg->state(), _server_in_box);
                    }
                }
                else if (interface == LabNetProt::INTERFACE_UART1)
                {
                    PinId id { interface::Interfaces::UART1, msg->id().pin() };

                    if (_pulseHelper.count(id) > 0)
                    {
                        so_5::send<JustSwitch>(_pulseHelper[id], msg->state());
                    }
                    else
                    {
                        so_5::send<interface::digital_messages::SetDigitalOut>(_uartBox, interface::Interfaces::UART1, msg->id().pin(), msg->state(), _server_in_box);
                    }
                }
                else if (interface == LabNetProt::INTERFACE_UART2)
                {
                    PinId id { interface::Interfaces::UART2, msg->id().pin() };

                    if (_pulseHelper.count(id) > 0)
                    {
                        so_5::send<JustSwitch>(_pulseHelper[id], msg->state());
                    }
                    else
                    {
                        so_5::send<interface::digital_messages::SetDigitalOut>(_uartBox, interface::Interfaces::UART2, msg->id().pin(), msg->state(), _server_in_box);
                    }
                }
                else if (interface == LabNetProt::INTERFACE_UART3)
                {
                    PinId id { interface::Interfaces::UART3, msg->id().pin() };

                    if (_pulseHelper.count(id) > 0)
                    {
                        so_5::send<JustSwitch>(_pulseHelper[id], msg->state());
                    }
                    else
                    {
                        so_5::send<interface::digital_messages::SetDigitalOut>(_uartBox, interface::Interfaces::UART3, msg->id().pin(), msg->state(), _server_in_box);
                    }
                }
                else if (interface == LabNetProt::INTERFACE_UART4)
                {
                    PinId id { interface::Interfaces::UART4, msg->id().pin() };

                    if (_pulseHelper.count(id) > 0)
                    {
                        so_5::send<JustSwitch>(_pulseHelper[id], msg->state());
                    }
                    else
                    {
                        so_5::send<interface::digital_messages::SetDigitalOut>(_uartBox, interface::Interfaces::UART4, msg->id().pin(), msg->state(), _server_in_box);
                    }
                }
                else if (interface == LabNetProt::INTERFACE_SOUND)
                {
                    PinId id { interface::Interfaces::SOUND, msg->id().pin() };

                    if (_pulseHelper.count(id) > 0)
                    {
                        so_5::send<JustSwitch>(_pulseHelper[id], msg->state());
                    }
                    else
                    {
                        so_5::send<interface::digital_messages::SetDigitalOut>(_soundBox, interface::Interfaces::SOUND, msg->id().pin(), msg->state(), _server_in_box);
                    }
                }
            })
        .event(_server_out_box,
            [this](std::shared_ptr<LabNetProt::Client::DigitalOutPulse> msg) {
                auto interface = msg->id().interface();
                if (interface == LabNetProt::INTERFACE_IO_BOARD)
                {
                    PinId id { interface::Interfaces::IO_BOARD, msg->id().pin() };

                    if (_pulseHelper.count(id) == 0)
                    {
                        auto coop = so_5::create_child_coop(*this);
                        auto a = coop->make_agent<PulseHelper>(_logger, _selfBox, _server_in_box, _gpioBox, id.interface, id.pin);

                        _pulseHelper[id] = a->so_direct_mbox();

                        so_environment().register_coop(std::move(coop));
                    }

                    so_5::send<StartPulse>(_pulseHelper[id], msg->high_duration(), msg->low_duration(), msg->pulses());
                }
                else if (interface == LabNetProt::INTERFACE_GPIO_WIRINGPI)
                {
                    PinId id { interface::Interfaces::GPIO_WIRING, msg->id().pin() };

                    if (_pulseHelper.count(id) == 0)
                    {
                        auto coop = so_5::create_child_coop(*this);
                        auto a = coop->make_agent<PulseHelper>(_logger, _selfBox, _server_in_box, _gpioWiringBox, id.interface, id.pin);

                        _pulseHelper[id] = a->so_direct_mbox();

                        so_environment().register_coop(std::move(coop));
                    }

                    so_5::send<StartPulse>(_pulseHelper[id], msg->high_duration(), msg->low_duration(), msg->pulses());
                }
                else if (interface == LabNetProt::INTERFACE_UART0
                    || interface == LabNetProt::INTERFACE_UART1
                    || interface == LabNetProt::INTERFACE_UART2
                    || interface == LabNetProt::INTERFACE_UART3
                    || interface == LabNetProt::INTERFACE_UART4)
                {
                    PinId id { static_cast<interface::Interfaces>(interface), msg->id().pin() };

                    if (_pulseHelper.count(id) == 0)
                    {
                        auto coop = so_5::create_child_coop(*this);
                        auto a = coop->make_agent<PulseHelper>(_logger, _selfBox, _server_in_box, _uartBox, id.interface, id.pin);

                        _pulseHelper[id] = a->so_direct_mbox();

                        so_environment().register_coop(std::move(coop));
                    }

                    so_5::send<StartPulse>(_pulseHelper[id], msg->high_duration(), msg->low_duration(), msg->pulses());
                }
                else if (interface == LabNetProt::INTERFACE_SOUND)
                {
                    PinId id { interface::Interfaces::SOUND, msg->id().pin() };

                    if (_pulseHelper.count(id) == 0)
                    {
                        auto coop = so_5::create_child_coop(*this);
                        auto a = coop->make_agent<PulseHelper>(_logger, _selfBox, _server_in_box, _soundBox, id.interface, id.pin);

                        _pulseHelper[id] = a->so_direct_mbox();

                        so_environment().register_coop(std::move(coop));
                    }

                    so_5::send<StartPulse>(_pulseHelper[id], msg->high_duration(), msg->low_duration(), msg->pulses());
                }
            })
        .event(_server_out_box,
            [this](std::shared_ptr<LabNetProt::Client::StartDigitalOutLoop> msg) {
                std::string loopName = msg->loop_name();

                if (loopName.size() > 0)
                {
                    if (_loopHelper.count(loopName) != 0)
                    {
                        so_5::send<AbortLoopHelper>(_loopHelper[loopName]);
                        _loopHelper.erase(loopName);
                    }

                    if (msg->digital_outputs_size() > 0)
                    {
                        auto coop = so_5::create_child_coop(*this);
                        auto a = coop->make_agent<LoopHelper>(_logger, _selfBox, _server_in_box);
                        _loopHelper[loopName] = a->so_direct_mbox();
                        so_environment().register_coop(std::move(coop));

                        so_5::send<LabNetProt::Client::StartDigitalOutLoop>(_loopHelper[loopName], *msg);
                    }
                    else
                    {
                        so_5::send<LoopStartFailed>(_server_in_box, loopName);
                    }
                }
            })
        .event(_server_out_box,
            [this](std::shared_ptr<LabNetProt::Client::StopDigitalOutLoop> msg) {
                auto loopName = msg->loop_name();

                if (_loopHelper.count(loopName) == 0)
                {
                }
                else
                {
                    so_5::send<LabNetProt::Client::StopDigitalOutLoop>(_loopHelper[loopName], *msg);
                    _loopHelper.erase(loopName);
                }
            });

    _pausedState
        .event(_server_out_box,
            [this](const mhood_t<interface::ContinueInterface>& msg) {
                this >>= _runningState;

                for (auto& loop : _loopHelper)
                {
                    so_5::send<ContinueLoopHelper>(loop.second);
                }
            });
}