#include <boost/format.hpp>
#include <chrono>

#include "../Interface/digital_messages.h"
#include "../Interface/interface_messages.h"
#include "../Interface/stream_messages.h"
#include "../helper/reset_msg.h"
#include "ProtocolAll.h"
#include "server_actor.h"
#include "server_messages.h"

using namespace LabNet::network;

server_actor::server_actor(context_t ctx, Logger logger)
    : so_5::agent_t(ctx)
    , _logger(logger)
    , _server_in_box(ctx.env().create_mbox("server_in"))
    , _server_out_box(ctx.env().create_mbox("server_out"))
{
}

server_actor::~server_actor()
{
}

void server_actor::so_define_agent()
{
    this >>= wait_for_connection_state;

    wait_for_connection_state
        .event(_server_in_box,
            [this](mhood_t<client_connected> mes) {
                _connection = mes->connection;
                so_5::send<interface::ContinueInterface>(_server_out_box);

                this >>= connected_state;
            });

    reset_state
        .event(_server_in_box,
            [this](mhood_t<client_disconnected>) {
                _connection.reset();

                this >>= reset_no_connection_state;
            })
        .event(_server_in_box,
            [this](mhood_t<LabNet::helper::ResetDone> msg) {
                std::shared_ptr<LabNetProt::Server::LabNetResetReply> resetReply = std::make_shared<LabNetProt::Server::LabNetResetReply>();
                _connection->send_message(resetReply, LabNetProt::Server::ServerMessageType::LABNET_RESET_REPLY);

                this >>= connected_state;
            });

    reset_no_connection_state
        .event(_server_in_box,
            [this](mhood_t<client_connected> mes) {
                _connection = mes->connection;

                this >>= reset_state;
            })
        .event(_server_in_box,
            [this](mhood_t<LabNet::helper::ResetDone> msg) {
                this >>= wait_for_connection_state;
            });

    connected_state
        .event(_server_in_box,
            [this](mhood_t<client_disconnected>) {
                _connection.reset();
                so_5::send<interface::PauseInterface>(_server_out_box);

                this >>= wait_for_connection_state;
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Client::LabNetResetRequest> mes) {
                so_5::send<LabNet::helper::StartReset>(_server_out_box);
                this >>= reset_state;
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Client::LabNetIdRequest> req) {
                std::shared_ptr<LabNetProt::Server::LabNetIdReply> reply = std::make_shared<LabNetProt::Server::LabNetIdReply>();
                reply->set_id("LabNet");
                reply->set_major_version(1);
                reply->set_minor_version(0);

                _connection->send_message(reply, LabNetProt::Server::ServerMessageType::LABNET_ID_REPLY);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Client::IoBoardInit> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::IoBoardInit>>(_server_out_box, mes);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalIn> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalIn>>(_server_out_box, mes);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalOut> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalOut>>(_server_out_box, mes);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Client::RfidBoardInit> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::RfidBoardInit>>(_server_out_box, mes);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Client::RfidBoardSetPhaseMatrix> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::RfidBoardSetPhaseMatrix>>(_server_out_box, mes);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Client::UartInit> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::UartInit>>(_server_out_box, mes);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Client::UartWriteData> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::UartWriteData>>(_server_out_box, mes);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Client::DigitalOutSet> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::DigitalOutSet>>(_server_out_box, mes);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Client::DigitalOutPulse> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::DigitalOutPulse>>(_server_out_box, mes);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Client::StartDigitalOutLoop> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::StartDigitalOutLoop>>(_server_out_box, mes);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Client::StopDigitalOutLoop> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::StopDigitalOutLoop>>(_server_out_box, mes);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInit> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::GpioWiringPiInit>>(_server_out_box, mes);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalIn> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalIn>>(_server_out_box, mes);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalOut> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalOut>>(_server_out_box, mes);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Client::InitSound> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::InitSound>>(_server_out_box, mes);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Client::DefineSineTone> mes) {
                so_5::send<std::shared_ptr<LabNetProt::Client::DefineSineTone>>(_server_out_box, mes);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Server::DigitalOutState> mes) {
                _connection->send_message(mes, LabNetProt::Server::ServerMessageType::DIGITAL_OUT_STATE);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Server::DigitalInState> mes) {
                _connection->send_message(mes, LabNetProt::Server::ServerMessageType::DIGITAL_IN_STATE);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Server::NewByteData> mes) {
                _connection->send_message(mes, LabNetProt::Server::ServerMessageType::NEW_BYTE_DATA);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Server::DataWriteComplete> mes) {
                _connection->send_message(mes, LabNetProt::Server::ServerMessageType::DATA_WRITE_COMPLETE);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Server::InterfaceInitResult> mes) {
                _connection->send_message(mes, LabNetProt::Server::ServerMessageType::INTERFACE_INIT_RESULT);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Server::DigitalInInitResult> mes) {
                _connection->send_message(mes, LabNetProt::Server::ServerMessageType::DIGITAL_IN_INIT_RESULT);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Server::DigitalOutInitResult> mes) {
                _connection->send_message(mes, LabNetProt::Server::ServerMessageType::DIGITAL_OUT_INIT_RESULT);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Server::LabNetResetReply> mes) {
                _connection->send_message(mes, LabNetProt::Server::ServerMessageType::LABNET_RESET_REPLY);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Server::InterfaceLost> mes) {
                _connection->send_message(mes, LabNetProt::Server::ServerMessageType::INTERFACE_LOST);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Server::InterfaceReconnected> mes) {
                _connection->send_message(mes, LabNetProt::Server::ServerMessageType::INTERFACE_RECONNECTED);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Server::DigitalOutLoopStartResult> mes) {
                _connection->send_message(mes, LabNetProt::Server::ServerMessageType::DIGITAL_OUT_LOOP_START_RESULT);
            })
        .event(_server_in_box,
            [this](std::shared_ptr<LabNetProt::Server::DigitalOutLoopStopped> mes) {
                _connection->send_message(mes, LabNetProt::Server::ServerMessageType::DIGITAL_OUT_LOOP_STOPPED);
            })
        .event(_server_in_box,
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

                _connection->send_message(digOutState, LabNetProt::Server::ServerMessageType::DIGITAL_OUT_STATE);
            })
        .event(_server_in_box,
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

                _connection->send_message(digInState, LabNetProt::Server::ServerMessageType::DIGITAL_IN_STATE);
            })
        .event(_server_in_box,
            [this](mhood_t<interface::InterfaceLost> mes) {
                std::shared_ptr<LabNetProt::Server::InterfaceLost> lost = std::make_shared<LabNetProt::Server::InterfaceLost>();
                lost->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));

                _connection->send_message(lost, LabNetProt::Server::ServerMessageType::INTERFACE_LOST);
            })
        .event(_server_in_box,
            [this](mhood_t<interface::InterfaceReconnected> mes) {
                std::shared_ptr<LabNetProt::Server::InterfaceReconnected> recon = std::make_shared<LabNetProt::Server::InterfaceReconnected>();
                recon->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));

                _connection->send_message(recon, LabNetProt::Server::ServerMessageType::INTERFACE_RECONNECTED);
            })
        .event(_server_in_box,
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

                _connection->send_message(data, LabNetProt::Server::ServerMessageType::NEW_BYTE_DATA);
            })
        .event(_server_in_box,
            [this](mhood_t<interface::stream_messages::SendDataComplete> mes) {
                std::shared_ptr<LabNetProt::Server::DataWriteComplete> write = std::make_shared<LabNetProt::Server::DataWriteComplete>();

                LabNetProt::PinId* id = new LabNetProt::PinId();
                id->set_pin(0);
                id->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));
                write->set_allocated_pin(id);

                _connection->send_message(write, LabNetProt::Server::ServerMessageType::DATA_WRITE_COMPLETE);
            })
        .event(_server_in_box,
            [this](const interface::digital_messages::DigitalInInitResult& msg) {
                std::shared_ptr<LabNetProt::Server::DigitalInInitResult> init = std::make_shared<LabNetProt::Server::DigitalInInitResult>();
                init->set_interface(static_cast<LabNetProt::Interfaces>(msg.interface));
                init->set_is_succeed(msg.is_succeed);
                init->set_pin(msg.pin);

                _connection->send_message(init, LabNetProt::Server::ServerMessageType::DIGITAL_IN_INIT_RESULT);
            })
        .event(_server_in_box,
            [this](const interface::digital_messages::DigitalOutInitResult& msg) {
                std::shared_ptr<LabNetProt::Server::DigitalOutInitResult> init = std::make_shared<LabNetProt::Server::DigitalOutInitResult>();
                init->set_interface(static_cast<LabNetProt::Interfaces>(msg.interface));
                init->set_is_succeed(msg.is_succeed);
                init->set_pin(msg.pin);

                _connection->send_message(init, LabNetProt::Server::ServerMessageType::DIGITAL_OUT_INIT_RESULT);
            });
}
