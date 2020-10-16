#include <chrono>
#include <memory>

#include "../LabNetMainActorMessages.h"
#include "../Network/LabNet.pb.h"
#include "../Network/LabNetClient.pb.h"
#include "DigitalMessages.h"
#include "InterfaceMessages.h"
#include "ManageInterfaces.h"
#include "io_board/GPIOManager.h"

Interface::ManageInterfaces::ManageInterfaces(context_t ctx, Logger logger, so_5::mbox_t labNetBox)
    : so_5::agent_t(ctx)
    , _self_mbox(ctx.env().create_mbox("ManageInterfaces"))
    , _logger(logger)
    , _labNetBox(labNetBox)
{
}

void Interface::ManageInterfaces::so_define_agent()
{
    so_subscribe(_self_mbox)
        .event([this](const mhood_t<continue_interface>& msg) {
            
        })
        .event([this](const mhood_t<pause_interface>& msg) {
            
        })
        .event([this](const mhood_t<stop_interface>& msg) {
            _rfid_board_init = false;
            _io_board_init = false;
            _gpio_wiring = false;
        })
        .event([this](std::shared_ptr<LabNetProt::Client::IoBoardInit> msg) {
            _logger->writeInfoEntry("IoBoardInit mes");
            if (_rfid_board_init || _gpio_wiring)
            {
                std::shared_ptr<LabNetProt::Server::InterfaceInitResult> initRes = std::make_shared<LabNetProt::Server::InterfaceInitResult>();
                initRes->set_interface(LabNetProt::INTERFACE_IO_BOARD);
                initRes->set_is_succeed(false);
                so_5::send<std::shared_ptr<LabNetProt::Server::InterfaceInitResult>>(_labNetBox, initRes);
            }
            else if (_io_board_init)
            {
                std::shared_ptr<LabNetProt::Server::InterfaceInitResult> initRes = std::make_shared<LabNetProt::Server::InterfaceInitResult>();
                initRes->set_interface(LabNetProt::INTERFACE_IO_BOARD);
                initRes->set_is_succeed(true);
                so_5::send<std::shared_ptr<LabNetProt::Server::InterfaceInitResult>>(_labNetBox, initRes);
            }
            else
            {
                _io_board_init = true;

                this->so_environment().introduce_coop([this](so_5::coop_t& coop) {
                    _gpioBox = coop.environment().create_mbox("gpio");

                    coop.make_agent<io_board::GPIOManager>(_gpioBox, _self_mbox, _self_mbox, _logger);
                });

                std::shared_ptr<LabNetProt::Server::InterfaceInitResult> initRes = std::make_shared<LabNetProt::Server::InterfaceInitResult>();
                initRes->set_interface(LabNetProt::INTERFACE_IO_BOARD);
                initRes->set_is_succeed(true);
                so_5::send<std::shared_ptr<LabNetProt::Server::InterfaceInitResult>>(_labNetBox, initRes);
            }
        })
        .event([this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalIn> msg) {
            if (_io_board_init)
            {
                so_5::send<std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalIn>>(_gpioBox, msg);
            }
        })
        .event([this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalOut> msg) {
            if (_io_board_init)
            {
                so_5::send<std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalOut>>(_gpioBox, msg);
            }
        })
        .event([this](const DigitalMessages::digital_in_init_result& msg) {
            std::shared_ptr<LabNetProt::Server::DigitalInInitResult> init = std::make_shared<LabNetProt::Server::DigitalInInitResult>();
            init->set_interface(static_cast<LabNetProt::Interfaces>(msg.interface));
            init->set_is_succeed(msg.is_succeed);
            init->set_pin(msg.pin);

            so_5::send<std::shared_ptr<LabNetProt::Server::DigitalInInitResult>>(_labNetBox, init);
        })
        .event([this](const DigitalMessages::digital_out_init_result& msg) {
            std::shared_ptr<LabNetProt::Server::DigitalOutInitResult> init = std::make_shared<LabNetProt::Server::DigitalOutInitResult>();
            init->set_interface(static_cast<LabNetProt::Interfaces>(msg.interface));
            init->set_is_succeed(msg.is_succeed);
            init->set_pin(msg.pin);

            so_5::send<std::shared_ptr<LabNetProt::Server::DigitalOutInitResult>>(_labNetBox, init);
        })
        .event([this](const DigitalMessages::return_digital_in_state& msg) {
            so_5::send<DigitalMessages::return_digital_in_state>(_labNetBox, msg);
        })
        .event([this](const DigitalMessages::invalid_digital_out_pin& msg) {

        })

        .event([this](std::shared_ptr<LabNetProt::Client::UartInit> msg) {
            _logger->writeInfoEntry("UartInit mes");
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

    LabNetProt::Client::UartInit uartInit;
    so_5::send<LabNet::RegisterForMessage>(_labNetBox, uartInit.GetTypeName(), _self_mbox);

    LabNetProt::Client::GpioWiringPiInit gpioWiringPiInit;
    so_5::send<LabNet::RegisterForMessage>(_labNetBox, gpioWiringPiInit.GetTypeName(), _self_mbox);

    so_5::send<LabNet::RegisterForMessage>(_labNetBox, std::string("pause_interface"), _self_mbox);
    so_5::send<LabNet::RegisterForMessage>(_labNetBox, std::string("reset_interface"), _self_mbox);
    so_5::send<LabNet::RegisterForMessage>(_labNetBox, std::string("continue_interface"), _self_mbox);
}