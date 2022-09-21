#include <boost/format.hpp>
#include <chrono>

#include "../interface/digital_messages.h"
#include "../interface/interface_messages.h"
#include "../interface/stream_messages.h"
#include "../helper/reset_msg.h"
#include "protocol_all.h"
#include "server_actor.h"
#include "server_messages.h"

using namespace LabNet::network;

ServerActor::ServerActor(context_t ctx, log::Logger logger)
    : so_5::agent_t(ctx)
    , logger_(logger)
    , server_in_box_(ctx.env().create_mbox("server_in"))
    , server_out_box_(ctx.env().create_mbox("server_out"))
{
}

ServerActor::~ServerActor()
{
}

void ServerActor::so_define_agent()
{
    this >>= wait_for_connection_state_;

    wait_for_connection_state_
        .event(server_in_box_,
            [this](mhood_t<ClientConnected> mes) {
                connection_ = mes->connection;
                so_5::send<interface::ContinueInterface>(server_out_box_);

                this >>= connected_state_;
            });

    reset_state_
        .event(server_in_box_,
            [this](mhood_t<ClientDisconnected>) {
                connection_.reset();

                this >>= reset_no_connection_state_;
            })
        .event(server_in_box_,
            [this](mhood_t<LabNet::helper::ResetDone> msg) {
                std::shared_ptr<LabNetProt::Server::LabNetResetReply> resetReply = std::make_shared<LabNetProt::Server::LabNetResetReply>();
                resetReply->set_is_reset(true);
                connection_->SendMessage(resetReply, LabNetProt::Server::ServerMessageType::LABNET_RESET_REPLY);

                this >>= connected_state_;
            });

    reset_no_connection_state_
        .event(server_in_box_,
            [this](mhood_t<ClientConnected> mes) {
                connection_ = mes->connection;

                this >>= reset_state_;
            })
        .event(server_in_box_,
            [this](mhood_t<LabNet::helper::ResetDone> msg) {
                this >>= wait_for_connection_state_;
            });

    connected_state_
        .event(server_in_box_,
            [this](mhood_t<ClientDisconnected>) {
                connection_.reset();
                so_5::send<interface::PauseInterface>(server_out_box_);

                this >>= wait_for_connection_state_;
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::LabNetResetRequest> mes) {
                so_5::send<LabNet::helper::StartReset>(server_out_box_);
                this >>= reset_state_;
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::LabNetIdRequest> req) {
                std::shared_ptr<LabNetProt::Server::LabNetIdReply> reply = std::make_shared<LabNetProt::Server::LabNetIdReply>();
                reply->set_id("LabNet");
                reply->set_major_version(1);
                reply->set_minor_version(0);

                connection_->SendMessage(reply, LabNetProt::Server::ServerMessageType::LABNET_ID_REPLY);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::IoBoardInit> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::IoBoardInit>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalIn> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalIn>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalOut> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalOut>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::RfidBoardInit> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::RfidBoardInit>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::RfidBoardSetPhaseMatrix> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::RfidBoardSetPhaseMatrix>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::UartInit> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::UartInit>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::UartWriteData> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::UartWriteData>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::DigitalOutSet> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::DigitalOutSet>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::DigitalOutPulse> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::DigitalOutPulse>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::StartDigitalOutLoop> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::StartDigitalOutLoop>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::StopDigitalOutLoop> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::StopDigitalOutLoop>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInit> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::GpioWiringPiInit>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalIn> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalIn>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalOut> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalOut>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::InitSound> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::InitSound>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::InitSoundSignal> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::InitSoundSignal>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::ChiBioInit> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::ChiBioInit>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::MoveChiBioPump> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::MoveChiBioPump>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::BleUartInit> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::BleUartInit>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::UartBoardInit> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::UartBoardInit>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::UartBoardWriteData> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::UartBoardWriteData>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Server::ChiBioPumpMoveResult> mes) {
                connection_->SendMessage(mes, LabNetProt::Server::ServerMessageType::CHI_BIO_PUMP_MOVE_RESULT);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::UartInitDigitalIn> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::UartInitDigitalIn>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Client::UartInitDigitalOut> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::UartInitDigitalOut>>(server_out_box_, mes);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Server::DigitalOutState> mes) {
                connection_->SendMessage(mes, LabNetProt::Server::ServerMessageType::DIGITAL_OUT_STATE);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Server::DigitalInState> mes) {
                connection_->SendMessage(mes, LabNetProt::Server::ServerMessageType::DIGITAL_IN_STATE);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Server::NewByteData> mes) {
                connection_->SendMessage(mes, LabNetProt::Server::ServerMessageType::NEW_BYTE_DATA);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Server::DataWriteComplete> mes) {
                connection_->SendMessage(mes, LabNetProt::Server::ServerMessageType::DATA_WRITE_COMPLETE);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Server::InterfaceInitResult> mes) {
                connection_->SendMessage(mes, LabNetProt::Server::ServerMessageType::INTERFACE_INIT_RESULT);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Server::DigitalInInitResult> mes) {
                connection_->SendMessage(mes, LabNetProt::Server::ServerMessageType::DIGITAL_IN_INIT_RESULT);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Server::DigitalOutInitResult> mes) {
                connection_->SendMessage(mes, LabNetProt::Server::ServerMessageType::DIGITAL_OUT_INIT_RESULT);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Server::LabNetResetReply> mes) {
                connection_->SendMessage(mes, LabNetProt::Server::ServerMessageType::LABNET_RESET_REPLY);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Server::InterfaceLost> mes) {
                connection_->SendMessage(mes, LabNetProt::Server::ServerMessageType::INTERFACE_LOST);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Server::InterfaceReconnected> mes) {
                connection_->SendMessage(mes, LabNetProt::Server::ServerMessageType::INTERFACE_RECONNECTED);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Server::DigitalOutLoopStartResult> mes) {
                connection_->SendMessage(mes, LabNetProt::Server::ServerMessageType::DIGITAL_OUT_LOOP_START_RESULT);
            })
        .event(server_in_box_,
            [this](std::shared_ptr<LabNetProt::Server::DigitalOutLoopStopped> mes) {
                connection_->SendMessage(mes, LabNetProt::Server::ServerMessageType::DIGITAL_OUT_LOOP_STOPPED);
            })
        .event(server_in_box_,
            [this](const mhood_t<interface::digital_messages::ReturnDigitalOutState>& mes) {
                std::shared_ptr<LabNetProt::Server::DigitalOutState> digOutState = std::make_shared<LabNetProt::Server::DigitalOutState>();
                LabNetProt::PinId* id = new LabNetProt::PinId();
                id->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));
                id->set_pin(mes->pin);
                digOutState->set_allocated_pin(id);
                digOutState->set_state(mes->state);

                using namespace std::chrono;
                google::protobuf::Timestamp* timestamp = new google::protobuf::Timestamp();
                nanoseconds ns = duration_cast<nanoseconds>(mes->time.time_since_epoch());
                seconds s = duration_cast<seconds>(ns);

                timestamp->set_seconds(s.count());
                timestamp->set_nanos(ns.count() % 1000000000);
                digOutState->set_allocated_time(timestamp);

                connection_->SendMessage(digOutState, LabNetProt::Server::ServerMessageType::DIGITAL_OUT_STATE);
            })
        .event(server_in_box_,
            [this](mhood_t<interface::digital_messages::ReturnDigitalInState> mes) {
                std::shared_ptr<LabNetProt::Server::DigitalInState> digInState = std::make_shared<LabNetProt::Server::DigitalInState>();
                LabNetProt::PinId* id = new LabNetProt::PinId();
                id->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));
                id->set_pin(mes->pin);
                digInState->set_allocated_pin(id);
                digInState->set_state(mes->state);

                using namespace std::chrono;
                google::protobuf::Timestamp* timestamp = new google::protobuf::Timestamp();
                nanoseconds ns = duration_cast<nanoseconds>(mes->time.time_since_epoch());
                seconds s = duration_cast<seconds>(ns);

                timestamp->set_seconds(s.count());
                timestamp->set_nanos(ns.count() % 1000000000);
                digInState->set_allocated_time(timestamp);

                connection_->SendMessage(digInState, LabNetProt::Server::ServerMessageType::DIGITAL_IN_STATE);
            })
        .event(server_in_box_,
            [this](mhood_t<interface::InterfaceLost> mes) {
                std::shared_ptr<LabNetProt::Server::InterfaceLost> lost = std::make_shared<LabNetProt::Server::InterfaceLost>();
                lost->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));

                connection_->SendMessage(lost, LabNetProt::Server::ServerMessageType::INTERFACE_LOST);
            })
        .event(server_in_box_,
            [this](mhood_t<interface::InterfaceReconnected> mes) {
                std::shared_ptr<LabNetProt::Server::InterfaceReconnected> recon = std::make_shared<LabNetProt::Server::InterfaceReconnected>();
                recon->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));

                connection_->SendMessage(recon, LabNetProt::Server::ServerMessageType::INTERFACE_RECONNECTED);
            })
        .event(server_in_box_,
            [this](mhood_t<interface::stream_messages::NewDataFromPort> mes) {
                std::shared_ptr<LabNetProt::Server::NewByteData> data = std::make_shared<LabNetProt::Server::NewByteData>();
                data->set_data(mes->data->data(), mes->data->size());

                LabNetProt::PinId* id = new LabNetProt::PinId();
                id->set_pin(mes->pin);
                id->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));
                data->set_allocated_pin(id);

                using namespace std::chrono;
                google::protobuf::Timestamp* timestamp = new google::protobuf::Timestamp();
                nanoseconds ns = duration_cast<nanoseconds>(mes->time.time_since_epoch());
                seconds s = duration_cast<seconds>(ns);

                timestamp->set_seconds(s.count());
                timestamp->set_nanos(ns.count() % 1000000000);
                data->set_allocated_time(timestamp);

                connection_->SendMessage(data, LabNetProt::Server::ServerMessageType::NEW_BYTE_DATA);
            })
        .event(server_in_box_,
            [this](mhood_t<interface::stream_messages::SendDataComplete> mes) {
                std::shared_ptr<LabNetProt::Server::DataWriteComplete> write = std::make_shared<LabNetProt::Server::DataWriteComplete>();

                LabNetProt::PinId* id = new LabNetProt::PinId();
                id->set_pin(0);
                id->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));
                write->set_allocated_pin(id);

                connection_->SendMessage(write, LabNetProt::Server::ServerMessageType::DATA_WRITE_COMPLETE);
            })
        .event(server_in_box_,
            [this](const interface::digital_messages::DigitalInInitResult& msg) {
                std::shared_ptr<LabNetProt::Server::DigitalInInitResult> init = std::make_shared<LabNetProt::Server::DigitalInInitResult>();
                init->set_interface(static_cast<LabNetProt::Interfaces>(msg.interface));
                init->set_is_succeed(msg.is_succeed);
                init->set_pin(msg.pin);

                connection_->SendMessage(init, LabNetProt::Server::ServerMessageType::DIGITAL_IN_INIT_RESULT);
            })
        .event(server_in_box_,
            [this](const interface::digital_messages::DigitalOutInitResult& msg) {
                std::shared_ptr<LabNetProt::Server::DigitalOutInitResult> init = std::make_shared<LabNetProt::Server::DigitalOutInitResult>();
                init->set_interface(static_cast<LabNetProt::Interfaces>(msg.interface));
                init->set_is_succeed(msg.is_succeed);
                init->set_pin(msg.pin);

                connection_->SendMessage(init, LabNetProt::Server::ServerMessageType::DIGITAL_OUT_INIT_RESULT);
            });
}
