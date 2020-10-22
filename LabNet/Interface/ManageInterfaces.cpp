#include <chrono>
#include <memory>

#include "../LabNetMainActorMessages.h"
#include "../Network/LabNet.pb.h"
#include "../Network/LabNetClient.pb.h"
#include "DigitalMessages.h"
#include "InterfaceMessages.h"
#include "ManageInterfaces.h"
#include "StreamMessages.h"
#include "gpio_wiring/GpioManager.h"
#include "io_board/GPIOManager.h"
#include "rfid_board/RfidMainActor.h"
#include "uart/SerialPortsManager.h"
#include "sound/SignalGenerator.h"

Interface::ManageInterfaces::ManageInterfaces(context_t ctx, Logger logger, so_5::mbox_t labNetBox)
    : so_5::agent_t(ctx)
    , _self_mbox(ctx.env().create_mbox("ManageInterfaces"))
    , _logger(logger)
    , _labNetBox(labNetBox)
{
    _reset_state[0] = false;
    _reset_state[1] = false;
    _reset_state[2] = false;
    _reset_state[3] = false;
    _reset_state[4] = false;
}

void Interface::ManageInterfaces::so_define_agent()
{
    this >>= running_state;

    running_state
        .event(_self_mbox,
            [this](const mhood_t<continue_interface>& msg) {
                if (_gpio_box)
                    so_5::send<Interface::continue_interface>(_gpio_box);
                if (_gpio_wiring_box)
                    so_5::send<Interface::continue_interface>(_gpio_wiring_box);
                if (_rfid_board_box)
                    so_5::send<Interface::continue_interface>(_rfid_board_box);
                if (_uart_box)
                    so_5::send<Interface::continue_interface>(_uart_box);
                if (_sound_box)
                    so_5::send<Interface::continue_interface>(_sound_box);
            })
        .event(_self_mbox,
            [this](const mhood_t<pause_interface>& msg) {
                if (_gpio_box)
                    so_5::send<Interface::pause_interface>(_gpio_box);
                if (_gpio_wiring_box)
                    so_5::send<Interface::pause_interface>(_gpio_wiring_box);
                if (_rfid_board_box)
                    so_5::send<Interface::pause_interface>(_rfid_board_box);
                if (_uart_box)
                    so_5::send<Interface::pause_interface>(_uart_box);
                if (_sound_box)
                    so_5::send<Interface::pause_interface>(_sound_box);
            })
        .event(_self_mbox,
            [this](const mhood_t<stop_interface>& msg) {
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
                    so_5::send<Interface::stop_interface>(_uart_box);
                }
                if (_sound_box)
                {
                    _reset_state[4] = true;
                    so_environment().deregister_coop(_sound_coop, so_5::dereg_reason::normal);
                }

                if (!is_reset_done())
                    this >>= reset_state;
            })
        .event(_self_mbox,
            [this](std::shared_ptr<LabNetProt::Client::IoBoardInit> msg) {
                if (_rfid_board_box || _gpio_wiring_box)
                {
                    std::shared_ptr<LabNetProt::Server::InterfaceInitResult> initRes = std::make_shared<LabNetProt::Server::InterfaceInitResult>();
                    initRes->set_interface(LabNetProt::INTERFACE_IO_BOARD);
                    initRes->set_is_succeed(false);
                    so_5::send<std::shared_ptr<LabNetProt::Server::InterfaceInitResult>>(_labNetBox, initRes);
                }
                else if (_gpio_box)
                {
                    std::shared_ptr<LabNetProt::Server::InterfaceInitResult> initRes = std::make_shared<LabNetProt::Server::InterfaceInitResult>();
                    initRes->set_interface(LabNetProt::INTERFACE_IO_BOARD);
                    initRes->set_is_succeed(true);
                    so_5::send<std::shared_ptr<LabNetProt::Server::InterfaceInitResult>>(_labNetBox, initRes);
                }
                else
                {
                    _gpio_box = so_environment().create_mbox("gpio");
                    auto gpio = so_environment().make_agent<io_board::GPIOManager>(_gpio_box, _self_mbox, _labNetBox, _logger);
                    _gpio_coop = so_environment().register_agent_as_coop(std::move(gpio));

                    so_5::send<io_board::init_io_board>(_gpio_box);
                }
            })
        .event(_self_mbox,
            [this](const mhood_t<Interface::interface_init_result> msg) {
                switch (msg->interface)
                {
                    case Interface::Interfaces::IO_BOARD:
                    {
                        if (!msg->is_succeed)
                        {
                            so_environment().deregister_coop(_gpio_coop, so_5::dereg_reason::normal);
                            _gpio_box = nullptr;
                        }
                    }
                    break;
                    case Interface::Interfaces::GPIO_WIRING:
                    {
                        if (!msg->is_succeed)
                        {
                            so_environment().deregister_coop(_gpio_wiring_coop, so_5::dereg_reason::normal);
                            _gpio_wiring_box = nullptr;
                        }
                    }
                    break;
                    case Interface::Interfaces::RFID_BOARD:
                    {
                        if (!msg->is_succeed)
                        {
                            so_environment().deregister_coop(_rfid_board_coop, so_5::dereg_reason::normal);
                            _rfid_board_box = nullptr;
                        }
                    }
                    case Interface::Interfaces::SOUND:
                    {
                        if (!msg->is_succeed)
                        {
                            so_environment().deregister_coop(_sound_coop, so_5::dereg_reason::normal);
                            _sound_box = nullptr;
                        }
                    }
                    break;
                    case Interface::Interfaces::UART0:
                    case Interface::Interfaces::UART1:
                    case Interface::Interfaces::UART2:
                    case Interface::Interfaces::UART3:
                    case Interface::Interfaces::UART4:
                        break;
                    default:
                        break;
                }

                std::shared_ptr<LabNetProt::Server::InterfaceInitResult> initRes = std::make_shared<LabNetProt::Server::InterfaceInitResult>();
                initRes->set_interface(static_cast<LabNetProt::Interfaces>(msg->interface));
                initRes->set_is_succeed(msg->is_succeed);
                so_5::send<std::shared_ptr<LabNetProt::Server::InterfaceInitResult>>(_labNetBox, initRes);
            })
        .event(_self_mbox,
            [this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalIn> msg) {
                if (_gpio_box)
                {
                    so_5::send<std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalIn>>(_gpio_box, msg);
                }
            })
        .event(_self_mbox,
            [this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalOut> msg) {
                if (_gpio_box)
                {
                    so_5::send<std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalOut>>(_gpio_box, msg);
                }
            })
        .event(_self_mbox,
            [this](const DigitalMessages::digital_in_init_result& msg) {
                std::shared_ptr<LabNetProt::Server::DigitalInInitResult> init = std::make_shared<LabNetProt::Server::DigitalInInitResult>();
                init->set_interface(static_cast<LabNetProt::Interfaces>(msg.interface));
                init->set_is_succeed(msg.is_succeed);
                init->set_pin(msg.pin);

                so_5::send<std::shared_ptr<LabNetProt::Server::DigitalInInitResult>>(_labNetBox, init);
            })
        .event(_self_mbox,
            [this](const DigitalMessages::digital_out_init_result& msg) {
                std::shared_ptr<LabNetProt::Server::DigitalOutInitResult> init = std::make_shared<LabNetProt::Server::DigitalOutInitResult>();
                init->set_interface(static_cast<LabNetProt::Interfaces>(msg.interface));
                init->set_is_succeed(msg.is_succeed);
                init->set_pin(msg.pin);

                so_5::send<std::shared_ptr<LabNetProt::Server::DigitalOutInitResult>>(_labNetBox, init);
            })
        .event(_self_mbox,
            [this](std::shared_ptr<LabNetProt::Client::RfidBoardInit> msg) {
                if (_gpio_box || _gpio_wiring_box)
                {
                    std::shared_ptr<LabNetProt::Server::InterfaceInitResult> initRes = std::make_shared<LabNetProt::Server::InterfaceInitResult>();
                    initRes->set_interface(LabNetProt::INTERFACE_RFID_BOARD);
                    initRes->set_is_succeed(false);
                    so_5::send<std::shared_ptr<LabNetProt::Server::InterfaceInitResult>>(_labNetBox, initRes);
                }
                else if (_rfid_board_box)
                {
                    std::shared_ptr<LabNetProt::Server::InterfaceInitResult> initRes = std::make_shared<LabNetProt::Server::InterfaceInitResult>();
                    initRes->set_interface(LabNetProt::INTERFACE_RFID_BOARD);
                    initRes->set_is_succeed(true);
                    so_5::send<std::shared_ptr<LabNetProt::Server::InterfaceInitResult>>(_labNetBox, initRes);
                }
                else
                {
                    _rfid_board_box = so_environment().create_mbox("rfid");
                    auto rfid_board = so_environment().make_agent<rfid_board::SamMainActor>(_rfid_board_box, _self_mbox, _labNetBox, _logger);
                    _rfid_board_coop = so_environment().register_agent_as_coop(std::move(rfid_board));

                    so_5::send<rfid_board::init_interface>(_rfid_board_box, msg->antenna_phase1(), msg->antenna_phase2(), msg->phase_duration(), msg->inverted());
                }
            })
        .event(_self_mbox,
            [this](std::shared_ptr<LabNetProt::Client::RfidBoardSetPhaseMatrix> msg) {
                if (_rfid_board_box)
                {
                    so_5::send<rfid_board::set_phase_matrix>(_rfid_board_box, msg->antenna_phase1(), msg->antenna_phase2(), msg->phase_duration());
                }
            })
        .event(_self_mbox,
            [this](std::shared_ptr<LabNetProt::Client::UartInit> msg) {
                if (!_uart_box)
                {
                    _uart_box = so_environment().create_mbox("uart");
                    auto uart = so_environment().make_agent<uart::SerialPortsManager>(_uart_box, _self_mbox, _labNetBox, _logger);
                    _uart_coop = so_environment().register_agent_as_coop(std::move(uart));
                }

                so_5::send<uart::init_serial_port>(_uart_box, static_cast<Interface::Interfaces>(msg->port()), msg->baud());
            })
        .event(_self_mbox,
            [this](std::shared_ptr<LabNetProt::Client::UartWriteData> msg) {
                if (_uart_box)
                {
                    so_5::send<std::shared_ptr<LabNetProt::Client::UartWriteData>>(_uart_box, msg);
                }
            })
        .event(_self_mbox,
            [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInit> msg) {
                if (_rfid_board_box || _gpio_box)
                {
                    std::shared_ptr<LabNetProt::Server::InterfaceInitResult> initRes = std::make_shared<LabNetProt::Server::InterfaceInitResult>();
                    initRes->set_interface(LabNetProt::INTERFACE_GPIO_WIRINGPI);
                    initRes->set_is_succeed(false);
                    so_5::send<std::shared_ptr<LabNetProt::Server::InterfaceInitResult>>(_labNetBox, initRes);
                }
                else if (_gpio_wiring_box)
                {
                    std::shared_ptr<LabNetProt::Server::InterfaceInitResult> initRes = std::make_shared<LabNetProt::Server::InterfaceInitResult>();
                    initRes->set_interface(LabNetProt::INTERFACE_GPIO_WIRINGPI);
                    initRes->set_is_succeed(true);
                    so_5::send<std::shared_ptr<LabNetProt::Server::InterfaceInitResult>>(_labNetBox, initRes);
                }
                else
                {
                    _gpio_wiring_box = so_environment().create_mbox("gpioWiring");
                    auto gpio = so_environment().make_agent<gpio_wiring::GpioManager>(_gpio_wiring_box, _self_mbox, _labNetBox, _logger);
                    _gpio_wiring_coop = so_environment().register_agent_as_coop(std::move(gpio));

                    so_5::send<gpio_wiring::init_gpio_wiring>(_gpio_wiring_box);
                }
            })
        .event(_self_mbox,
            [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalIn> msg) {
                if (_gpio_wiring_box)
                {
                    so_5::send<std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalIn>>(_gpio_wiring_box, msg);
                }
            })
        .event(_self_mbox,
            [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalOut> msg) {
                if (_gpio_wiring_box)
                {
                    so_5::send<std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalOut>>(_gpio_wiring_box, msg);
                }
            })
        .event(_self_mbox,
            [this](const so_5::mhood_t<interface_lost>& msg) {
                so_5::send<interface_lost>(_labNetBox, msg->interface);
            })
        .event(_self_mbox,
            [this](const so_5::mhood_t<interface_reconnected>& msg) {
                so_5::send<interface_reconnected>(_labNetBox, msg->interface);
            })
        .event(_self_mbox,
            [this](std::shared_ptr<LabNetProt::Client::InitSound> msg) {
                if (!_sound_box)
                {
                    _sound_box = so_environment().create_mbox("sound");
                    auto sound = so_environment().make_agent<sound::SignalGenerator>(_sound_box, _self_mbox, _labNetBox, _logger);
                    _sound_coop = so_environment().register_agent_as_coop(std::move(sound));
                }

                so_5::send<sound::init_sound>(_sound_box);
            })
        .event(_self_mbox,
            [this](std::shared_ptr<LabNetProt::Client::DefineSineTone> msg) {
                if (_sound_box)
                {
                    so_5::send<std::shared_ptr<LabNetProt::Client::DefineSineTone>>(_sound_box, msg);
                }
            });

    reset_state
        .event(_self_mbox,
            [this](const mhood_t<Interface::interface_stopped> msg) {
                switch (msg->interface)
                {
                    case Interface::Interfaces::IO_BOARD:
                    {
                        _gpio_box = nullptr;
                        _reset_state[0] = false;
                    }
                    break;
                    case Interface::Interfaces::GPIO_WIRING:
                    {
                        _gpio_wiring_box = nullptr;
                        _reset_state[1] = false;
                    }
                    break;
                    case Interface::Interfaces::RFID_BOARD:
                    {
                        _rfid_board_box = nullptr;
                        _reset_state[2] = false;
                    }
                    break;
                    case Interface::Interfaces::SOUND:
                    {
                        _sound_box = nullptr;
                        _reset_state[4] = false;
                    }
                    break;
                    case Interface::Interfaces::UART0:
                    case Interface::Interfaces::UART1:
                    case Interface::Interfaces::UART2:
                    case Interface::Interfaces::UART3:
                    case Interface::Interfaces::UART4:
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

void Interface::ManageInterfaces::so_evt_start()
{
    LabNetProt::Client::IoBoardInit ioBordInit;
    so_5::send<LabNet::RegisterForMessage>(_labNetBox, ioBordInit.GetTypeName(), _self_mbox);

    LabNetProt::Client::IoBoardInitDigitalIn ioBordInitDigIn;
    so_5::send<LabNet::RegisterForMessage>(_labNetBox, ioBordInitDigIn.GetTypeName(), _self_mbox);

    LabNetProt::Client::IoBoardInitDigitalOut ioBordInitDigOut;
    so_5::send<LabNet::RegisterForMessage>(_labNetBox, ioBordInitDigOut.GetTypeName(), _self_mbox);

    LabNetProt::Client::RfidBoardInit rfidBordInit;
    so_5::send<LabNet::RegisterForMessage>(_labNetBox, rfidBordInit.GetTypeName(), _self_mbox);

    LabNetProt::Client::RfidBoardSetPhaseMatrix rfidSetPhaseMatrix;
    so_5::send<LabNet::RegisterForMessage>(_labNetBox, rfidSetPhaseMatrix.GetTypeName(), _self_mbox);

    LabNetProt::Client::UartInit uartInit;
    so_5::send<LabNet::RegisterForMessage>(_labNetBox, uartInit.GetTypeName(), _self_mbox);

    LabNetProt::Client::UartWriteData uartWrite;
    so_5::send<LabNet::RegisterForMessage>(_labNetBox, uartWrite.GetTypeName(), _self_mbox);

    LabNetProt::Client::GpioWiringPiInit gpioWiringPiInit;
    so_5::send<LabNet::RegisterForMessage>(_labNetBox, gpioWiringPiInit.GetTypeName(), _self_mbox);

    LabNetProt::Client::GpioWiringPiInitDigitalIn gpioWiringPiInitDigIn;
    so_5::send<LabNet::RegisterForMessage>(_labNetBox, gpioWiringPiInitDigIn.GetTypeName(), _self_mbox);

    LabNetProt::Client::GpioWiringPiInitDigitalOut gpioWiringPiInitDigOut;
    so_5::send<LabNet::RegisterForMessage>(_labNetBox, gpioWiringPiInitDigOut.GetTypeName(), _self_mbox);

    LabNetProt::Client::InitSound initSound;
    so_5::send<LabNet::RegisterForMessage>(_labNetBox, initSound.GetTypeName(), _self_mbox);

    LabNetProt::Client::DefineSineTone defineSineTone;
    so_5::send<LabNet::RegisterForMessage>(_labNetBox, defineSineTone.GetTypeName(), _self_mbox);

    so_5::send<LabNet::RegisterForMessage>(_labNetBox, std::string("pause_interface"), _self_mbox);
    so_5::send<LabNet::RegisterForMessage>(_labNetBox, std::string("stop_interface"), _self_mbox);
    so_5::send<LabNet::RegisterForMessage>(_labNetBox, std::string("continue_interface"), _self_mbox);
}

bool Interface::ManageInterfaces::is_reset_done()
{
    for (auto& out : _reset_state)
    {
        if (out.second)
            return false;
    }

    so_5::send<Interface::reset_done>(_labNetBox, _self_mbox);

    return true;
}