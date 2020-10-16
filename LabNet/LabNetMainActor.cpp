#include <boost/format.hpp>
#include <chrono>

#include "Interface/InterfaceMessages.h"
#include "Interface/DigitalMessages.h"
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
        });
    reset_no_connection_state
        .event([this](mhood_t<ClientConnected> mes) {
            _connection = mes->connection;

            this >>= reset_state;
        });

    connected_state
        .event([this](mhood_t<ClientDisconnected>) {
            _connection.reset();
            Interface::pause_interface pi;
            send_message(pi, "pause_interface");

            this >>= wait_for_connection_state;
        })
        .event([this](std::shared_ptr<LabNetProt::Client::LabNetResetRequest> mes) {
            Interface::stop_interface stop;
            send_message(stop, "reset_interface");

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
        .event([this](std::shared_ptr<LabNetProt::Client::RfidBoardSetSignalInversion> mes) {
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
        //.event([this](std::shared_ptr<LabNetProt::Client::ClientWrappedMessage> mes) {
        //	switch (mes->client_message_case())
        //	{
        //	case LabNetProt::Client::ClientWrappedMessage::kIoBoardInit:
        //	{
        //		auto& ioBoardInit = mes->io_board_init();
        //		send_message(ioBoardInit);
        //		//so_5::send<io_board::init_interface>(_gpioBox);
        //	}
        //		break;
        //	case LabNetProt::Client::ClientWrappedMessage::kIoBoardInitDigitalIn:
        //	{
        //		auto& init_in = mes->io_board_init_digital_in();
        //		send_message(init_in);
        //		//so_5::send<io_board::init_digital_in>(_gpioBox, init_in.pin(), static_cast<io_board::resistor>(init_in.resistor_state()), init_in.is_inverted());
        //	}
        //		break;
        //	case LabNetProt::Client::ClientWrappedMessage::kIoBoardInitDigitalOut:
        //		{
        //			auto& init_out = mes->io_board_init_digital_out();
        //			send_message(init_out);
        //			//so_5::send<io_board::init_digital_out>(_gpioBox, init_out.pin(), init_out.is_inverted());
        //		}
        //		break;
        //	case LabNetProt::Client::ClientWrappedMessage::kRfidBoardInit:
        //		{
        //			auto& init_sam = mes->rfid_board_init();
        //			send_message(init_sam);
        //			//so_5::send<rfid_board::init_interface>(_rfidBox, init_sam.antenna_phase1(), init_sam.antenna_phase2(), init_sam.phase_duration(), init_sam.inverted());
        //		}
        //		break;
        //	case LabNetProt::Client::ClientWrappedMessage::kRfidBoardSetPhaseMatrix:
        //		{
        //			auto& set_phase = mes->rfid_board_set_phase_matrix();
        //			send_message(set_phase);
        //			//so_5::send<rfid_board::set_phase_matrix>(_rfidBox, set_phase.antenna_phase1(), set_phase.antenna_phase2(), set_phase.phase_duration());
        //		}
        //		break;
        //	case LabNetProt::Client::ClientWrappedMessage::kRfidBoardSetSignalInversion:
        //		{
        //			auto& set_inv = mes->rfid_board_set_signal_inversion();
        //			send_message(set_inv);
        //			//so_5::send<rfid_board::set_signal_inversion>(_rfidBox, set_inv.inverted());
        //		}
        //		break;
        //	case LabNetProt::Client::ClientWrappedMessage::kUartInit:
        //		{
        //			_logger->writeInfoEntry("uart init mes");
        //			//auto& uart_init = mes->uart_init();
        //			std::shared_ptr<LabNetProt::Client::UartInit> m(std::move( mes->uart_init()));
        //			send_message(std::move( mes->uart_init()));
        //			//so_5::send<uart::messages::init_port>(_uartBox, static_cast<Interface::Interfaces>(uart_init.port()), uart_init.baud());
        //		}
        //		break;
        //	case LabNetProt::Client::ClientWrappedMessage::kUartWriteData:
        //		{
        //			auto& uart_write = mes->uart_write_data();
        //			send_message(uart_write);
        //			/*
        //			std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>();
        //			data->resize(uart_write.data().size());
        //			for (int l = 0; l < uart_write.data().size(); l++)
        //				data->push_back(uart_write.data()[l]);
        //
        //			so_5::send<StreamMessages::send_data_to_port>(_uartBox, static_cast<Interface::Interfaces>(uart_write.port()), 0, data);
        //			*/
        //		}
        //		break;
        //	case LabNetProt::Client::ClientWrappedMessage::kGpioWiringPiInit:
        //		{
        //			auto& gpioWiringPiInit = mes->gpio_wiring_pi_init();
        //			send_message(gpioWiringPiInit);
        //			//so_5::send<gpio_wiring::init_interface>(_gpioWiringBox);
        //		}
        //		break;
        //	case LabNetProt::Client::ClientWrappedMessage::kGpioWiringPiInitDigitalIn:
        //		{
        //			auto& init_in = mes->gpio_wiring_pi_init_digital_in();
        //			send_message(init_in);
        //			//so_5::send<gpio_wiring::init_digital_in>(_gpioWiringBox, init_in.pin(), static_cast<gpio_wiring::resistor>(init_in.resistor_state()), init_in.is_inverted());
        //		}
        //		break;
        //	case LabNetProt::Client::ClientWrappedMessage::kGpioWiringPiInitDigitalOut:
        //		{
        //			auto& init_in = mes->gpio_wiring_pi_init_digital_out();
        //			send_message(init_in);
        //			//so_5::send<gpio_wiring::init_digital_out>(_gpioWiringBox, init_in.pin(), init_in.is_inverted());
        //		}
        //		break;
        //	case LabNetProt::Client::ClientWrappedMessage::kDigitalOutSet:
        //		{
        //			auto set = mes->digital_out_set();
        //			send_message(set);
        //			//so_5::send<LabNetProt::Client::DigitalOutSet>(_digOutBox, set);
        //		}
        //		break;
        //	case LabNetProt::Client::ClientWrappedMessage::kDigitalOutPulse:
        //		{
        //			auto setPulse = mes->digital_out_pulse();
        //			send_message(setPulse);
        //			//so_5::send<LabNetProt::Client::DigitalOutPulse>(_digOutBox, setPulse);
        //		}
        //		break;
        //	case LabNetProt::Client::ClientWrappedMessage::kStartDigitalOutLoop:
        //		{
        //			auto setLoop = mes->start_digital_out_loop();
        //			send_message(setLoop);
        //			//so_5::send<LabNetProt::Client::StartDigitalOutLoop>(_digOutBox, setLoop);
        //		}
        //		break;
        //	case LabNetProt::Client::ClientWrappedMessage::kStopDigitalOutLoop:
        //		{
        //			auto stopLoop = mes->stop_digital_out_loop();
        //			send_message(stopLoop);
        //			//so_5::send<LabNetProt::Client::StopDigitalOutLoop>(_digOutBox, stopLoop);
        //		}
        //		break;
        //	case LabNetProt::Client::ClientWrappedMessage::kResetRequest:
        //		{
        //			auto& reset = mes->reset_request();
        //			send_message(reset);
        //			/*
        //			so_5::send<Interface::reset_interface>(_gpioBox);
        //			so_5::send<Interface::reset_interface>(_interfaceManager);
        //			so_5::send<Interface::reset_interface>(_uartBox);
        //			so_5::send<Interface::reset_interface>(_gpioWiringBox);
        //			*/
        //			std::shared_ptr<LabNetProt::Server::ServerWrappedMessage> swm = std::make_shared<LabNetProt::Server::ServerWrappedMessage>();
        //			LabNetProt::Server::LabNetResetReply* reply = new LabNetProt::Server::LabNetResetReply();
        //			reply->set_is_reset(true);
        //			swm->set_allocated_reset(reply);
        //
        //			_connection->send_message(swm);
        //
        //		}
        //		break;
        //	case LabNetProt::Client::ClientWrappedMessage::kIdRequest:
        //		{
        //			_logger->writeInfoEntry("id mes");
        //
        //			std::shared_ptr<LabNetProt::Server::ServerWrappedMessage> swm = std::make_shared<LabNetProt::Server::ServerWrappedMessage>();
        //			LabNetProt::Server::LabNetIdReply* id = new LabNetProt::Server::LabNetIdReply();
        //			id->set_id("LabNet");
        //			id->set_major_version(1);
        //			id->set_minor_version(0);
        //			swm->set_allocated_id(id);
        //			_connection->send_message(swm);
        //		}
        //		break;
        //	case LabNetProt::Client::ClientWrappedMessage::CLIENT_MESSAGE_NOT_SET:
        //		break;
        //	}
        //})
        //.event([this](std::shared_ptr<LabNetProt::Server::ServerWrappedMessage> mes) {
        //	_connection->send_message(mes);
        //})
        /*
	.event([this](mhood_t<DigitalMessages::digital_in_init_result> mes)
	{
		std::shared_ptr<LabNetProt::Server::ServerWrappedMessage> swm = std::make_shared<LabNetProt::Server::ServerWrappedMessage>();
		LabNetProt::Server::DigitalInInitResult* res = new LabNetProt::Server::DigitalInInitResult();
		res->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));
		res->set_pin(mes->pin);
		res->set_is_succeed(mes->is_succeed);
			
		swm->set_allocated_digital_in_init_result(res);
		
		_connection->send_message(swm);
	})
	.event([this](mhood_t<DigitalMessages::return_digital_in_state> mes) {
		std::shared_ptr<LabNetProt::Server::ServerWrappedMessage> swm = std::make_shared<LabNetProt::Server::ServerWrappedMessage>();
		LabNetProt::Server::DigitalInState* state = new LabNetProt::Server::DigitalInState();
		LabNetProt::PinId *id = new LabNetProt::PinId();
		id->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));
		id->set_pin(mes->pin);
		state->set_allocated_pin(id);
		state->set_state(mes->state);
		
		using namespace std::chrono;
		google::protobuf::Timestamp* timestamp = new google::protobuf::Timestamp();
		nanoseconds ns = duration_cast<nanoseconds>(mes->time.time_since_epoch());
		seconds s = duration_cast<seconds>(ns);
		
		timestamp->set_seconds(s.count());
		timestamp->set_nanos(ns.count() % 1000000000);
		state->set_allocated_time(timestamp);
			
		swm->set_allocated_digital_in_state(state);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<DigitalMessages::digital_out_init_result> mes)
	{
		std::shared_ptr<LabNetProt::Server::ServerWrappedMessage> swm = std::make_shared<LabNetProt::Server::ServerWrappedMessage>();
		LabNetProt::Server::DigitalOutInitResult* res = new LabNetProt::Server::DigitalOutInitResult();
		res->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));
		res->set_pin(mes->pin);
		res->set_is_succeed(mes->is_succeed);
			
		swm->set_allocated_digital_out_init_result(res);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<DigitalMessages::return_digital_out_state> mes)
	{
		std::shared_ptr<LabNetProt::Server::ServerWrappedMessage> swm = std::make_shared<LabNetProt::Server::ServerWrappedMessage>();
		LabNetProt::Server::DigitalOutState* state = new LabNetProt::Server::DigitalOutState();
		LabNetProt::PinId *id = new LabNetProt::PinId();
		id->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));
		id->set_pin(mes->pin);
		state->set_allocated_pin(id);
		state->set_state(mes->state);
		
		using namespace std::chrono;
		google::protobuf::Timestamp* timestamp = new google::protobuf::Timestamp();
		nanoseconds ns = duration_cast<nanoseconds>(mes->time.time_since_epoch());
		seconds s = duration_cast<seconds>(ns);
		
		timestamp->set_seconds(s.count());
		timestamp->set_nanos(ns.count() % 1000000000);
		state->set_allocated_time(timestamp);
		
		swm->set_allocated_digital_out_state(state);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<Interface::interface_lost> mes)
	{
		std::shared_ptr<LabNetProt::Server::ServerWrappedMessage> swm = std::make_shared<LabNetProt::Server::ServerWrappedMessage>();
		LabNetProt::Server::InterfaceLost* lost = new LabNetProt::Server::InterfaceLost();
		lost->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));
		
		swm->set_allocated_interface_lost(lost);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<Interface::interface_reconnected> mes)
	{
		std::shared_ptr<LabNetProt::Server::ServerWrappedMessage> swm = std::make_shared<LabNetProt::Server::ServerWrappedMessage>();
		LabNetProt::Server::InterfaceReconnected* lost = new LabNetProt::Server::InterfaceReconnected();
		lost->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));
		
		swm->set_allocated_interface_reconnected(lost);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<StreamMessages::new_data_from_port> mes)
	{
		std::shared_ptr<LabNetProt::Server::ServerWrappedMessage> swm = std::make_shared<LabNetProt::Server::ServerWrappedMessage>();
		LabNetProt::Server::NewByteData* data = new LabNetProt::Server::NewByteData();
		
		data->set_data(mes->data->data(), mes->data->size());
		
		LabNetProt::PinId *id = new LabNetProt::PinId();
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
		
		swm->set_allocated_new_byte_data(data);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<StreamMessages::send_data_complete> mes)
	{
		std::shared_ptr<LabNetProt::Server::ServerWrappedMessage> swm = std::make_shared<LabNetProt::Server::ServerWrappedMessage>();
		LabNetProt::Server::DataWriteComplete* write = new LabNetProt::Server::DataWriteComplete();
		
		LabNetProt::PinId *id = new LabNetProt::PinId();
		id->set_pin(0);
		id->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));
		write->set_allocated_pin(id);
		
		swm->set_allocated_data_write_complete(write);
		_connection->send_message(swm);
	})
	*/
        ;
}

/*
void LabNetMainActor::send_message(google::protobuf::Message& mes)
{
	std::string mes_name = mes.GetTypeName();
	auto it = _receiver.find(mes_name);
	if (it != _receiver.end())
	{
		so_5::send(it->second, mes);
	}
}
*/