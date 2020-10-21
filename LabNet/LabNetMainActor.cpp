#include <boost/format.hpp>
#include <chrono>

#include "../Interface/StreamMessages.h"
#include "Interface/DigitalMessages.h"
#include "Interface/InterfaceMessages.h"
#include "LabNetMainActor.h"
#include "LabNetMainActorMessages.h"
#include "Network/ProtocolAll.h"

using namespace LabNet;

LabNetMainActor::LabNetMainActor(context_t ctx, Logger logger)
    : so_5::agent_t(ctx)
    , _logger(logger)
    , _receiver()
{
}

LabNetMainActor::~LabNetMainActor()
{
}

void LabNetMainActor::so_define_agent()
{
    this >>= wait_for_connection_state;

    wait_for_connection_state
        .event([this](mhood_t<ClientConnected> mes) {
            _connection = mes->connection;
            Interface::continue_interface ci;
            send_message(ci, "continue_interface");

            this >>= connected_state;
        })
        .event([this](mhood_t<RegisterForMessage> mes) {
            auto it = _receiver.find(mes->message_name);
            if (it == _receiver.end())
            {
                _receiver[mes->message_name] = std::vector<so_5::mbox_t>();
            }

            _receiver[mes->message_name].push_back(mes->receiver);

            /*boost::format fmt = boost::format("receiver for: %s") % mes->message_name;
            _logger->writeInfoEntry(fmt.str());*/
        });

    reset_state
        .event([this](mhood_t<ClientDisconnected>) {
            _connection.reset();

            this >>= reset_no_connection_state;
        })
        .event([this](mhood_t<Interface::reset_done> msg) {
            _reset_request[msg->mbox] = false;

            bool done = true;
            for (auto& rec : _reset_request)
            {
                if (rec.second)
                {
                    done = false;
                    break;
                }
            }

            if (done)
            {
                std::shared_ptr<LabNetProt::Server::LabNetResetReply> resetReply = std::make_shared<LabNetProt::Server::LabNetResetReply>();
                _connection->send_message(resetReply, LabNetProt::Server::ServerMessageType::LABNET_RESET_REPLY);

                this >>= connected_state;
            }
        });
    reset_no_connection_state
        .event([this](mhood_t<ClientConnected> mes) {
            _connection = mes->connection;

            this >>= reset_state;
        })
        .event([this](mhood_t<Interface::reset_done> msg) {
            _reset_request[msg->mbox] = false;

            bool done = true;
            for (auto& rec : _reset_request)
            {
                if (rec.second)
                {
                    done = false;
                    break;
                }
            }

            if (done)
            {
                this >>= wait_for_connection_state;
            }
        });

    connected_state
        .event([this](mhood_t<ClientDisconnected>) {
            _connection.reset();
            Interface::pause_interface pi;
            send_message(pi, "pause_interface");

            this >>= wait_for_connection_state;
        })
        .event([this](std::shared_ptr<LabNetProt::Client::LabNetResetRequest> mes) {
            _reset_request.clear();

            Interface::stop_interface stop;
            auto it = _receiver.find("stop_interface");
            if (it != _receiver.end())
            {
                for (auto& rec : it->second)
                {
                    _reset_request[rec] = true;
                    so_5::send<Interface::stop_interface>(rec, stop);
                }
            }

            if (_reset_request.size() > 0)
                this >>= reset_state;
        })
        .event([this](std::shared_ptr<LabNetProt::Client::LabNetIdRequest> req) {
            std::shared_ptr<LabNetProt::Server::LabNetIdReply> reply = std::make_shared<LabNetProt::Server::LabNetIdReply>();
            reply->set_id("LabNetN");
            reply->set_major_version(1);
            reply->set_minor_version(0);

            _connection->send_message(reply, LabNetProt::Server::ServerMessageType::LABNET_ID_REPLY);
        })
        .event([this](std::shared_ptr<LabNetProt::Client::IoBoardInit> mes) {
            send_message(mes);
        })
        .event([this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalIn> mes) {
            send_message(mes);
        })
        .event([this](std::shared_ptr<LabNetProt::Client::IoBoardInitDigitalOut> mes) {
            send_message(mes);
        })
        .event([this](std::shared_ptr<LabNetProt::Client::RfidBoardInit> mes) {
            send_message(mes);
        })
        .event([this](std::shared_ptr<LabNetProt::Client::RfidBoardSetPhaseMatrix> mes) {
            send_message(mes);
        })
        .event([this](std::shared_ptr<LabNetProt::Client::UartInit> mes) {
            send_message(mes);
        })
        .event([this](std::shared_ptr<LabNetProt::Client::UartWriteData> mes) {
            send_message(mes);
        })
        .event([this](std::shared_ptr<LabNetProt::Client::DigitalOutSet> mes) {
            send_message(mes);
        })
        .event([this](std::shared_ptr<LabNetProt::Client::DigitalOutPulse> mes) {
            send_message(mes);
        })
        .event([this](std::shared_ptr<LabNetProt::Client::StartDigitalOutLoop> mes) {
            send_message(mes);
        })
        .event([this](std::shared_ptr<LabNetProt::Client::StopDigitalOutLoop> mes) {
            send_message(mes);
        })
        .event([this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInit> mes) {
            send_message(mes);
        })
        .event([this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalIn> mes) {
            send_message(mes);
        })
        .event([this](std::shared_ptr<LabNetProt::Client::GpioWiringPiInitDigitalOut> mes) {
            send_message(mes);
        })
        .event([this](std::shared_ptr<LabNetProt::Server::DigitalOutState> mes) {
            _connection->send_message(mes, LabNetProt::Server::ServerMessageType::DIGITAL_OUT_STATE);
        })
        .event([this](std::shared_ptr<LabNetProt::Server::DigitalInState> mes) {
            _connection->send_message(mes, LabNetProt::Server::ServerMessageType::DIGITAL_IN_STATE);
        })
        .event([this](std::shared_ptr<LabNetProt::Server::NewByteData> mes) {
            _connection->send_message(mes, LabNetProt::Server::ServerMessageType::NEW_BYTE_DATA);
        })
        .event([this](std::shared_ptr<LabNetProt::Server::DataWriteComplete> mes) {
            _connection->send_message(mes, LabNetProt::Server::ServerMessageType::DATA_WRITE_COMPLETE);
        })
        .event([this](std::shared_ptr<LabNetProt::Server::InterfaceInitResult> mes) {
            _connection->send_message(mes, LabNetProt::Server::ServerMessageType::INTERFACE_INIT_RESULT);
        })
        .event([this](std::shared_ptr<LabNetProt::Server::DigitalInInitResult> mes) {
            _connection->send_message(mes, LabNetProt::Server::ServerMessageType::DIGITAL_IN_INIT_RESULT);
        })
        .event([this](std::shared_ptr<LabNetProt::Server::DigitalOutInitResult> mes) {
            _connection->send_message(mes, LabNetProt::Server::ServerMessageType::DIGITAL_OUT_INIT_RESULT);
        })
        .event([this](std::shared_ptr<LabNetProt::Server::LabNetResetReply> mes) {
            _connection->send_message(mes, LabNetProt::Server::ServerMessageType::LABNET_RESET_REPLY);
        })
        .event([this](std::shared_ptr<LabNetProt::Server::InterfaceLost> mes) {
            _connection->send_message(mes, LabNetProt::Server::ServerMessageType::INTERFACE_LOST);
        })
        .event([this](std::shared_ptr<LabNetProt::Server::InterfaceReconnected> mes) {
            _connection->send_message(mes, LabNetProt::Server::ServerMessageType::INTERFACE_RECONNECTED);
        })
        .event([this](std::shared_ptr<LabNetProt::Server::DigitalOutLoopStartResult> mes) {
            _connection->send_message(mes, LabNetProt::Server::ServerMessageType::DIGITAL_OUT_LOOP_START_RESULT);
        })
        .event([this](std::shared_ptr<LabNetProt::Server::DigitalOutLoopStopped> mes) {
            _connection->send_message(mes, LabNetProt::Server::ServerMessageType::DIGITAL_OUT_LOOP_STOPPED);
        })
        .event([this](const mhood_t<DigitalMessages::return_digital_out_state>& mes) {
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
        .event([this](mhood_t<DigitalMessages::return_digital_in_state> mes) {
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
        .event([this](mhood_t<Interface::interface_lost> mes) {
            std::shared_ptr<LabNetProt::Server::InterfaceLost> lost = std::make_shared<LabNetProt::Server::InterfaceLost>();
            lost->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));

            _connection->send_message(lost, LabNetProt::Server::ServerMessageType::INTERFACE_LOST);
        })
        .event([this](mhood_t<Interface::interface_reconnected> mes) {
            std::shared_ptr<LabNetProt::Server::InterfaceReconnected> recon = std::make_shared<LabNetProt::Server::InterfaceReconnected>();
            recon->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));

            _connection->send_message(recon, LabNetProt::Server::ServerMessageType::INTERFACE_RECONNECTED);
        })
        .event([this](mhood_t<StreamMessages::new_data_from_port> mes) {
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
        .event([this](mhood_t<StreamMessages::send_data_complete> mes) {
            std::shared_ptr<LabNetProt::Server::DataWriteComplete> write = std::make_shared<LabNetProt::Server::DataWriteComplete>();

            LabNetProt::PinId* id = new LabNetProt::PinId();
            id->set_pin(0);
            id->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));
            write->set_allocated_pin(id);

            _connection->send_message(write, LabNetProt::Server::ServerMessageType::DATA_WRITE_COMPLETE);
        })
        .event([this](const DigitalMessages::digital_in_init_result& msg) {
            std::shared_ptr<LabNetProt::Server::DigitalInInitResult> init = std::make_shared<LabNetProt::Server::DigitalInInitResult>();
            init->set_interface(static_cast<LabNetProt::Interfaces>(msg.interface));
            init->set_is_succeed(msg.is_succeed);
            init->set_pin(msg.pin);

            _connection->send_message(init, LabNetProt::Server::ServerMessageType::DIGITAL_IN_INIT_RESULT);
        })
        .event([this](const DigitalMessages::digital_out_init_result& msg) {
            std::shared_ptr<LabNetProt::Server::DigitalOutInitResult> init = std::make_shared<LabNetProt::Server::DigitalOutInitResult>();
            init->set_interface(static_cast<LabNetProt::Interfaces>(msg.interface));
            init->set_is_succeed(msg.is_succeed);
            init->set_pin(msg.pin);

            _connection->send_message(init, LabNetProt::Server::ServerMessageType::DIGITAL_OUT_INIT_RESULT);
        })
        ;
}
