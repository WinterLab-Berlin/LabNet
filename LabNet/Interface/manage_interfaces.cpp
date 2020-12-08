#include <chrono>
#include <memory>

#include "../helper/reset_msg.h"
#include "../network/LabNet.pb.h"
#include "../network/LabNetClient.pb.h"
#include "../network/server_messages.h"
#include "digital_messages.h"
#include "interface_messages.h"
#include "manage_interfaces.h"
#include "stream_messages.h"
#include "gpio_wiring/gpio_manager.h"
#include "io_board/board_actor.h"
#include "rfid_board/rfid_main_actor.h"
#include "sound/signal_generator.h"
#include "uart/serial_ports_manager.h"

namespace LabNet::interface
{
    ManageInterfaces::ManageInterfaces(context_t ctx, Logger logger)
        : so_5::agent_t(ctx)
        , _self_mbox(ctx.env().create_mbox("manage_interfaces"))
        , _server_out_box(ctx.env().create_mbox("server_out"))
        , _server_in_box(ctx.env().create_mbox("server_in"))
        , _logger(logger)
    {
        _reset_state[0] = false;
        _reset_state[1] = false;
        _reset_state[2] = false;
        _reset_state[3] = false;
        _reset_state[4] = false;
    }

    void ManageInterfaces::so_define_agent()
    {
        this >>= running_state;

        running_state
            .event(_server_out_box,
                [this](const mhood_t<ContinueInterface>& msg) {
                    if (_gpio_box)
                        so_5::send<ContinueInterface>(_gpio_box);
                    if (_gpio_wiring_box)
                        so_5::send<ContinueInterface>(_gpio_wiring_box);
                    if (_rfid_board_box)
                        so_5::send<ContinueInterface>(_rfid_board_box);
                    if (_uart_box)
                        so_5::send<ContinueInterface>(_uart_box);
                    if (_sound_box)
                        so_5::send<ContinueInterface>(_sound_box);
                })
            .event(_server_out_box,
                [this](const mhood_t<PauseInterface>& msg) {
                    if (_gpio_box)
                        so_5::send<PauseInterface>(_gpio_box);
                    if (_gpio_wiring_box)
                        so_5::send<PauseInterface>(_gpio_wiring_box);
                    if (_rfid_board_box)
                        so_5::send<PauseInterface>(_rfid_board_box);
                    if (_uart_box)
                        so_5::send<PauseInterface>(_uart_box);
                    if (_sound_box)
                        so_5::send<PauseInterface>(_sound_box);
                })
            .event(_self_mbox,
                [this](const mhood_t<LabNet::helper::ResetRequest>& msg) {
                    if (_gpio_box)
                    {
                        _reset_state[0] = true;
                        so_environment().deregister_coop(_gpio_coop, so_5::dereg_reason::normal);
                    }
                    if (_gpio_wiring_box)
                    {
                        _reset_state[1] = true;
                        so_environment().deregister_coop(_gpio_wiring_coop, so_5::dereg_reason::normal);
                    }
                    if (_rfid_board_box)
                    {
                        _reset_state[2] = true;
                        so_environment().deregister_coop(_rfid_board_coop, so_5::dereg_reason::normal);
                    }
                    if (_uart_box)
                    {
                        _reset_state[3] = true;
                        so_5::send<StopInterface>(_uart_box);
                    }
                    if (_sound_box)
                    {
                        _reset_state[4] = true;
                        so_environment().deregister_coop(_sound_coop, so_5::dereg_reason::normal);
                    }

                    _reset_response_box = msg->response_box;
                    if (!is_reset_done())
                        this >>= reset_state;
                })
            .event(_server_out_box,
                [this](std::shared_ptr<LabNetProt::Client::IoBoardInit> msg) {
                    if (!_gpio_box)
                    {
                        _gpio_box = so_environment().create_mbox("gpio");
                        auto gpio = so_environment().make_agent<io_board::BoardActor>(_gpio_box, _self_mbox, _server_in_box, _logger);
                        _gpio_coop = so_environment().register_agent_as_coop(std::move(gpio));
                    }

                    so_5::send<io_board::init_io_board>(_gpio_box);
                })
            .event(_self_mbox,
                [this](const mhood_t<InterfaceInitResult> msg) {
                    switch (msg->interface)
                    {
                        case Interfaces::IO_BOARD:
                        {
                            if (!msg->is_succeed)
                            {
                                so_environment().deregister_coop(_gpio_coop, so_5::dereg_reason::normal);
                                _gpio_box = nullptr;
                            }
                        }
                        break;
                        case Interfaces::GPIO_WIRING:
                        {
                            if (!msg->is_succeed)
                            {
                                so_environment().deregister_coop(_gpio_wiring_coop, so_5::dereg_reason::normal);
                                _gpio_wiring_box = nullptr;
                            }
                        }
                        break;
                        case Interfaces::RFID_BOARD:
                        {
                            if (!msg->is_succeed)
                            {
                                so_environment().deregister_coop(_rfid_board_coop, so_5::dereg_reason::normal);
                                _rfid_board_box = nullptr;
                            }
                        }
                        case Interfaces::SOUND:
                        {
                            if (!msg->is_succeed)
                            {
                                so_environment().deregister_coop(_sound_coop, so_5::dereg_reason::normal);
                                _sound_box = nullptr;
                            }
                        }
                        break;
                        case Interfaces::UART0:
                        case Interfaces::UART1:
                        case Interfaces::UART2:
                        case Interfaces::UART3:
                        case Interfaces::UART4:
                            break;
                        default:
                            break;
                    }

                    std::shared_ptr<LabNetProt::Server::InterfaceInitResult> initRes = std::make_shared<LabNetProt::Server::InterfaceInitResult>();
                    initRes->set_interface(static_cast<LabNetProt::Interfaces>(msg->interface));
                    initRes->set_is_succeed(msg->is_succeed);
                    so_5::send<std::shared_ptr<LabNetProt::Server::InterfaceInitResult>>(_server_in_box, initRes);
                })
            .event(_server_out_box,
                [this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalIn> msg) {
                    if (_gpio_box)
                    {
                        so_5::send<std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalIn>>(_gpio_box, msg);
                    }
                })
            .event(_server_out_box,
                [this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalOut> msg) {
                    if (_gpio_box)
                    {
                        so_5::send<std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalOut>>(_gpio_box, msg);
                    }
                })
            .event(_self_mbox,
                [this](const digital_messages::DigitalInInitResult& msg) {
                    std::shared_ptr<LabNetProt::Server::DigitalInInitResult> init = std::make_shared<LabNetProt::Server::DigitalInInitResult>();
                    init->set_interface(static_cast<LabNetProt::Interfaces>(msg.interface));
                    init->set_is_succeed(msg.is_succeed);
                    init->set_pin(msg.pin);

                    so_5::send<std::shared_ptr<LabNetProt::Server::DigitalInInitResult>>(_server_in_box, init);
                })
            .event(_self_mbox,
                [this](const digital_messages::DigitalOutInitResult& msg) {
                    std::shared_ptr<LabNetProt::Server::DigitalOutInitResult> init = std::make_shared<LabNetProt::Server::DigitalOutInitResult>();
                    init->set_interface(static_cast<LabNetProt::Interfaces>(msg.interface));
                    init->set_is_succeed(msg.is_succeed);
                    init->set_pin(msg.pin);

                    so_5::send<std::shared_ptr<LabNetProt::Server::DigitalOutInitResult>>(_server_in_box, init);
                })
            .event(_server_out_box,
                [this](std::shared_ptr<LabNetProt::Client::RfidBoardInit> msg) {
                    if (!_rfid_board_box)
                    {
                        _rfid_board_box = so_environment().create_mbox("rfid");
                        auto rfid_board = so_environment().make_agent<rfid_board::RfidBoardMainActor>(_rfid_board_box, _self_mbox, _server_in_box, _logger);
                        _rfid_board_coop = so_environment().register_agent_as_coop(std::move(rfid_board));
                    }

                    so_5::send<rfid_board::init_interface>(_rfid_board_box, msg->antenna_phase1(), msg->antenna_phase2(), msg->phase_duration(), msg->inverted());
                })
            .event(_server_out_box,
                [this](std::shared_ptr<LabNetProt::Client::RfidBoardSetPhaseMatrix> msg) {
                    if (_rfid_board_box)
                    {
                        so_5::send<rfid_board::set_phase_matrix>(_rfid_board_box, msg->antenna_phase1(), msg->antenna_phase2(), msg->phase_duration());
                    }
                })
            .event(_server_out_box,
                [this](std::shared_ptr<LabNetProt::Client::UartInit> msg) {
                    if (!_uart_box)
                    {
                        _uart_box = so_environment().create_mbox("uart");
                        auto uart = so_environment().make_agent<uart::SerialPortsManager>(_uart_box, _self_mbox, _server_in_box, _logger);
                        _uart_coop = so_environment().register_agent_as_coop(std::move(uart));
                    }

                    so_5::send<uart::InitSerialPort>(_uart_box, static_cast<Interfaces>(msg->port()), msg->baud());
                })
            .event(_server_out_box,
                [this](std::shared_ptr<LabNetProt::Client::UartWriteData> msg) {
                    if (_uart_box)
                    {
                        so_5::send<std::shared_ptr<LabNetProt::Client::UartWriteData>>(_uart_box, msg);
                    }
                })
            .event(_server_out_box,
                [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInit> msg) {
                    if (!_gpio_wiring_box)
                    {
                        _gpio_wiring_box = so_environment().create_mbox("gpioWiring");
                        auto gpio = so_environment().make_agent<gpio_wiring::GpioManager>(_gpio_wiring_box, _self_mbox, _server_in_box, _logger);
                        _gpio_wiring_coop = so_environment().register_agent_as_coop(std::move(gpio));
                    }

                    so_5::send<gpio_wiring::init_gpio_wiring>(_gpio_wiring_box);
                })
            .event(_server_out_box,
                [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalIn> msg) {
                    if (_gpio_wiring_box)
                    {
                        so_5::send<std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalIn>>(_gpio_wiring_box, msg);
                    }
                })
            .event(_server_out_box,
                [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalOut> msg) {
                    if (_gpio_wiring_box)
                    {
                        so_5::send<std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalOut>>(_gpio_wiring_box, msg);
                    }
                })
            .event(_self_mbox,
                [this](const so_5::mhood_t<InterfaceLost>& msg) {
                    so_5::send<InterfaceLost>(_server_in_box, msg->interface);
                })
            .event(_self_mbox,
                [this](const so_5::mhood_t<InterfaceReconnected>& msg) {
                    so_5::send<InterfaceReconnected>(_server_in_box, msg->interface);
                })
            .event(_server_out_box,
                [this](std::shared_ptr<LabNetProt::Client::InitSound> msg) {
                    if (!_sound_box)
                    {
                        _sound_box = so_environment().create_mbox("sound");
                        auto sound = so_environment().make_agent<sound::SignalGenerator>(_sound_box, _self_mbox, _server_in_box, _logger);
                        _sound_coop = so_environment().register_agent_as_coop(std::move(sound));
                    }

                    so_5::send<sound::init_sound>(_sound_box);
                })
            .event(_server_out_box,
                [this](std::shared_ptr<LabNetProt::Client::DefineSineTone> msg) {
                    if (_sound_box)
                    {
                        so_5::send<std::shared_ptr<LabNetProt::Client::DefineSineTone>>(_sound_box, msg);
                    }
                });

        reset_state
            .event(_self_mbox,
                [this](const mhood_t<InterfaceStopped> msg) {
                    switch (msg->interface)
                    {
                        case Interfaces::IO_BOARD:
                        {
                            _gpio_box = nullptr;
                            _reset_state[0] = false;
                        }
                        break;
                        case Interfaces::GPIO_WIRING:
                        {
                            _gpio_wiring_box = nullptr;
                            _reset_state[1] = false;
                        }
                        break;
                        case Interfaces::RFID_BOARD:
                        {
                            _rfid_board_box = nullptr;
                            _reset_state[2] = false;
                        }
                        break;
                        case Interfaces::SOUND:
                        {
                            _sound_box = nullptr;
                            _reset_state[4] = false;
                        }
                        break;
                        case Interfaces::UART0:
                        case Interfaces::UART1:
                        case Interfaces::UART2:
                        case Interfaces::UART3:
                        case Interfaces::UART4:
                        {
                            _reset_state[3] = false;
                        }
                        break;
                        default:
                            break;
                    }

                    if (is_reset_done())
                        this >>= running_state;
                });
    }

    void ManageInterfaces::so_evt_start()
    {
    }

    bool ManageInterfaces::is_reset_done()
    {
        for (auto& out : _reset_state)
        {
            if (out.second)
                return false;
        }

        so_5::send<LabNet::helper::ResetDoneResponse>(_reset_response_box, LabNet::helper::ResponseId::ManageInterfaces);

        return true;
    }
};