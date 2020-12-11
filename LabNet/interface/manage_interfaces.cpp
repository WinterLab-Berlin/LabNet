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
    ManageInterfaces::ManageInterfaces(context_t ctx, log::Logger logger)
        : so_5::agent_t(ctx)
        , self_mbox_(ctx.env().create_mbox("manage_interfaces"))
        , server_out_box_(ctx.env().create_mbox("server_out"))
        , server_in_box_(ctx.env().create_mbox("server_in"))
        , logger_(logger)
    {
        reset_states_[0] = false;
        reset_states_[1] = false;
        reset_states_[2] = false;
        reset_states_[3] = false;
        reset_states_[4] = false;
    }

    void ManageInterfaces::so_define_agent()
    {
        this >>= running_state_;

        running_state_
            .event(server_out_box_,
                [this](const mhood_t<ContinueInterface>& msg) {
                    if (gpio_box_)
                        so_5::send<ContinueInterface>(gpio_box_);
                    if (gpio_wiring_box_)
                        so_5::send<ContinueInterface>(gpio_wiring_box_);
                    if (rfid_board_box_)
                        so_5::send<ContinueInterface>(rfid_board_box_);
                    if (uart_box_)
                        so_5::send<ContinueInterface>(uart_box_);
                    if (sound_box_)
                        so_5::send<ContinueInterface>(sound_box_);
                })
            .event(server_out_box_,
                [this](const mhood_t<PauseInterface>& msg) {
                    if (gpio_box_)
                        so_5::send<PauseInterface>(gpio_box_);
                    if (gpio_wiring_box_)
                        so_5::send<PauseInterface>(gpio_wiring_box_);
                    if (rfid_board_box_)
                        so_5::send<PauseInterface>(rfid_board_box_);
                    if (uart_box_)
                        so_5::send<PauseInterface>(uart_box_);
                    if (sound_box_)
                        so_5::send<PauseInterface>(sound_box_);
                })
            .event(self_mbox_,
                [this](const mhood_t<LabNet::helper::ResetRequest>& msg) {
                    if (gpio_box_)
                    {
                        reset_states_[0] = true;
                        so_environment().deregister_coop(gpio_coop_, so_5::dereg_reason::normal);
                    }
                    if (gpio_wiring_box_)
                    {
                        reset_states_[1] = true;
                        so_environment().deregister_coop(gpio_wiring_coop_, so_5::dereg_reason::normal);
                    }
                    if (rfid_board_box_)
                    {
                        reset_states_[2] = true;
                        so_environment().deregister_coop(rfid_board_coop_, so_5::dereg_reason::normal);
                    }
                    if (uart_box_)
                    {
                        reset_states_[3] = true;
                        so_5::send<StopInterface>(uart_box_);
                    }
                    if (sound_box_)
                    {
                        reset_states_[4] = true;
                        so_environment().deregister_coop(sound_coop_, so_5::dereg_reason::normal);
                    }

                    reset_response_box_ = msg->response_box;
                    if (!is_reset_done())
                        this >>= reset_state_;
                })
            .event(server_out_box_,
                [this](std::shared_ptr<LabNetProt::Client::IoBoardInit> msg) {
                    if (!gpio_box_)
                    {
                        gpio_box_ = so_environment().create_mbox("gpio");
                        auto gpio = so_environment().make_agent<io_board::BoardActor>(gpio_box_, self_mbox_, server_in_box_, logger_);
                        gpio_coop_ = so_environment().register_agent_as_coop(std::move(gpio));
                    }

                    so_5::send<io_board::InitIoBoard>(gpio_box_);
                })
            .event(self_mbox_,
                [this](const mhood_t<InterfaceInitResult> msg) {
                    switch (msg->interface)
                    {
                        case Interfaces::IoBoard:
                        {
                            if (!msg->is_succeed)
                            {
                                so_environment().deregister_coop(gpio_coop_, so_5::dereg_reason::normal);
                                gpio_box_ = nullptr;
                            }
                        }
                        break;
                        case Interfaces::GpioWiring:
                        {
                            if (!msg->is_succeed)
                            {
                                so_environment().deregister_coop(gpio_wiring_coop_, so_5::dereg_reason::normal);
                                gpio_wiring_box_ = nullptr;
                            }
                        }
                        break;
                        case Interfaces::RfidBoard:
                        {
                            if (!msg->is_succeed)
                            {
                                so_environment().deregister_coop(rfid_board_coop_, so_5::dereg_reason::normal);
                                rfid_board_box_ = nullptr;
                            }
                        }
                        case Interfaces::Sound:
                        {
                            if (!msg->is_succeed)
                            {
                                so_environment().deregister_coop(sound_coop_, so_5::dereg_reason::normal);
                                sound_box_ = nullptr;
                            }
                        }
                        break;
                        case Interfaces::Uart0:
                        case Interfaces::Uart1:
                        case Interfaces::Uart2:
                        case Interfaces::Uart3:
                        case Interfaces::Uart4:
                            break;
                        default:
                            break;
                    }

                    std::shared_ptr<LabNetProt::Server::InterfaceInitResult> initRes = std::make_shared<LabNetProt::Server::InterfaceInitResult>();
                    initRes->set_interface(static_cast<LabNetProt::Interfaces>(msg->interface));
                    initRes->set_is_succeed(msg->is_succeed);
                    so_5::send<std::shared_ptr<LabNetProt::Server::InterfaceInitResult>>(server_in_box_, initRes);
                })
            .event(server_out_box_,
                [this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalIn> msg) {
                    if (gpio_box_)
                    {
                        so_5::send<std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalIn>>(gpio_box_, msg);
                    }
                })
            .event(server_out_box_,
                [this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalOut> msg) {
                    if (gpio_box_)
                    {
                        so_5::send<std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalOut>>(gpio_box_, msg);
                    }
                })
            .event(self_mbox_,
                [this](const digital_messages::DigitalInInitResult& msg) {
                    std::shared_ptr<LabNetProt::Server::DigitalInInitResult> init = std::make_shared<LabNetProt::Server::DigitalInInitResult>();
                    init->set_interface(static_cast<LabNetProt::Interfaces>(msg.interface));
                    init->set_is_succeed(msg.is_succeed);
                    init->set_pin(msg.pin);

                    so_5::send<std::shared_ptr<LabNetProt::Server::DigitalInInitResult>>(server_in_box_, init);
                })
            .event(self_mbox_,
                [this](const digital_messages::DigitalOutInitResult& msg) {
                    std::shared_ptr<LabNetProt::Server::DigitalOutInitResult> init = std::make_shared<LabNetProt::Server::DigitalOutInitResult>();
                    init->set_interface(static_cast<LabNetProt::Interfaces>(msg.interface));
                    init->set_is_succeed(msg.is_succeed);
                    init->set_pin(msg.pin);

                    so_5::send<std::shared_ptr<LabNetProt::Server::DigitalOutInitResult>>(server_in_box_, init);
                })
            .event(server_out_box_,
                [this](std::shared_ptr<LabNetProt::Client::RfidBoardInit> msg) {
                    if (!rfid_board_box_)
                    {
                        rfid_board_box_ = so_environment().create_mbox("rfid");
                        auto rfid_board = so_environment().make_agent<rfid_board::RfidBoardMainActor>(rfid_board_box_, self_mbox_, server_in_box_, logger_);
                        rfid_board_coop_ = so_environment().register_agent_as_coop(std::move(rfid_board));
                    }

                    so_5::send<rfid_board::InitRfidBoard>(rfid_board_box_, msg->antenna_phase1(), msg->antenna_phase2(), msg->phase_duration(), msg->inverted());
                })
            .event(server_out_box_,
                [this](std::shared_ptr<LabNetProt::Client::RfidBoardSetPhaseMatrix> msg) {
                    if (rfid_board_box_)
                    {
                        so_5::send<rfid_board::SetPhaseMatrix>(rfid_board_box_, msg->antenna_phase1(), msg->antenna_phase2(), msg->phase_duration());
                    }
                })
            .event(server_out_box_,
                [this](std::shared_ptr<LabNetProt::Client::UartInit> msg) {
                    if (!uart_box_)
                    {
                        uart_box_ = so_environment().create_mbox("uart");
                        auto uart = so_environment().make_agent<uart::SerialPortsManager>(uart_box_, self_mbox_, server_in_box_, logger_);
                        uart_coop_ = so_environment().register_agent_as_coop(std::move(uart));
                    }

                    so_5::send<uart::InitSerialPort>(uart_box_, static_cast<Interfaces>(msg->port()), msg->baud());
                })
            .event(server_out_box_,
                [this](std::shared_ptr<LabNetProt::Client::UartWriteData> msg) {
                    if (uart_box_)
                    {
                        so_5::send<std::shared_ptr<LabNetProt::Client::UartWriteData>>(uart_box_, msg);
                    }
                })
            .event(server_out_box_,
                [this](std::shared_ptr<LabNetProt::Client::UartInitDigitalIn> msg) {
                    if (uart_box_)
                    {
                        so_5::send<std::shared_ptr<LabNetProt::Client::UartInitDigitalIn>>(uart_box_, msg);
                    }
                })
            .event(server_out_box_,
                [this](std::shared_ptr<LabNetProt::Client::UartInitDigitalOut> msg) {
                    if (uart_box_)
                    {
                        so_5::send<std::shared_ptr<LabNetProt::Client::UartInitDigitalOut>>(uart_box_, msg);
                    }
                })
            .event(server_out_box_,
                [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInit> msg) {
                    if (!gpio_wiring_box_)
                    {
                        gpio_wiring_box_ = so_environment().create_mbox("gpioWiring");
                        auto gpio = so_environment().make_agent<gpio_wiring::GpioManager>(gpio_wiring_box_, self_mbox_, server_in_box_, logger_);
                        gpio_wiring_coop_ = so_environment().register_agent_as_coop(std::move(gpio));
                    }

                    so_5::send<gpio_wiring::InitGpioWiring>(gpio_wiring_box_);
                })
            .event(server_out_box_,
                [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalIn> msg) {
                    if (gpio_wiring_box_)
                    {
                        so_5::send<std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalIn>>(gpio_wiring_box_, msg);
                    }
                })
            .event(server_out_box_,
                [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalOut> msg) {
                    if (gpio_wiring_box_)
                    {
                        so_5::send<std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalOut>>(gpio_wiring_box_, msg);
                    }
                })
            .event(self_mbox_,
                [this](const so_5::mhood_t<InterfaceLost>& msg) {
                    so_5::send<InterfaceLost>(server_in_box_, msg->interface);
                })
            .event(self_mbox_,
                [this](const so_5::mhood_t<InterfaceReconnected>& msg) {
                    so_5::send<InterfaceReconnected>(server_in_box_, msg->interface);
                })
            .event(server_out_box_,
                [this](std::shared_ptr<LabNetProt::Client::InitSound> msg) {
                    if (!sound_box_)
                    {
                        sound_box_ = so_environment().create_mbox("sound");
                        auto sound = so_environment().make_agent<sound::SignalGenerator>(sound_box_, self_mbox_, server_in_box_, logger_);
                        sound_coop_ = so_environment().register_agent_as_coop(std::move(sound));
                    }

                    so_5::send<sound::init_sound>(sound_box_);
                })
            .event(server_out_box_,
                [this](std::shared_ptr<LabNetProt::Client::DefineSineTone> msg) {
                    if (sound_box_)
                    {
                        so_5::send<std::shared_ptr<LabNetProt::Client::DefineSineTone>>(sound_box_, msg);
                    }
                });

        reset_state_
            .event(self_mbox_,
                [this](const mhood_t<InterfaceStopped> msg) {
                    switch (msg->interface)
                    {
                        case Interfaces::IoBoard:
                        {
                            gpio_box_ = nullptr;
                            reset_states_[0] = false;
                        }
                        break;
                        case Interfaces::GpioWiring:
                        {
                            gpio_wiring_box_ = nullptr;
                            reset_states_[1] = false;
                        }
                        break;
                        case Interfaces::RfidBoard:
                        {
                            rfid_board_box_ = nullptr;
                            reset_states_[2] = false;
                        }
                        break;
                        case Interfaces::Sound:
                        {
                            sound_box_ = nullptr;
                            reset_states_[4] = false;
                        }
                        break;
                        case Interfaces::Uart0:
                        case Interfaces::Uart1:
                        case Interfaces::Uart2:
                        case Interfaces::Uart3:
                        case Interfaces::Uart4:
                        {
                            reset_states_[3] = false;
                        }
                        break;
                        default:
                            break;
                    }

                    if (is_reset_done())
                        this >>= running_state_;
                });
    }

    void ManageInterfaces::so_evt_start()
    {
    }

    bool ManageInterfaces::is_reset_done()
    {
        for (auto& out : reset_states_)
        {
            if (out.second)
                return false;
        }

        so_5::send<LabNet::helper::ResetDoneResponse>(reset_response_box_, LabNet::helper::ResponseId::ManageInterfaces);

        return true;
    }
};