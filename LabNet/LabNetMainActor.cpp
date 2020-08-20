#include "LabNetMainActor.h"
#include "LabNetMainActorMessages.h"
#include "Network/ProtocolAll.h"
#include "Interface/io_board/Messages.h"
#include "Interface/InterfaceMessages.h"
#include "Interface/rfid_board/RfidMessages.h"
#include "Interface/UART/SerialPortMessages.h"
#include "Interface/DigitalMessages.h"
#include "Interface/StreamMessages.h"
#include <chrono>

using namespace LabNet;

LabNetMainActor::LabNetMainActor(context_t ctx, Logger logger, so_5::mbox_t gpioBox, so_5::mbox_t rfidBox, so_5::mbox_t uartBox, so_5::mbox_t digOutBox)
	: so_5::agent_t(ctx)
	, _logger(logger)
	, _gpioBox(gpioBox)
	, _rfidBox(rfidBox)
	, _uartBox(uartBox)
	, _digOutBox(digOutBox)
	, _interfaceManager(ctx.env().create_mbox("ManageInterfaces"))
{
}

LabNetMainActor::~LabNetMainActor()
{
	
}

void LabNetMainActor::so_define_agent()
{
	this >>= wait_for_connection;

	wait_for_connection
		.event([this](mhood_t<Connected> mes) {
			_connection = mes->connection;
			
			so_5::send<Interface::continue_interface>(_gpioBox);
			so_5::send<Interface::continue_interface>(_rfidBox);
			so_5::send<Interface::continue_interface>(_uartBox);
			so_5::send<Interface::continue_interface>(_digOutBox);
			
			this >>= connected;
		});
	
	connected
		.event([this](mhood_t<Disconnected>) {
		_connection.reset();
			
		so_5::send<Interface::pause_interface>(_gpioBox);
			so_5::send<Interface::pause_interface>(_rfidBox);
		so_5::send<Interface::pause_interface>(_uartBox);
		so_5::send<Interface::pause_interface>(_digOutBox);
			
		this >>= wait_for_connection;
	})
	.event([this](std::shared_ptr<LabNetProt::Client::ClientWrappedMessage> mes) {
		switch (mes->client_message_case())
		{
		case LabNetProt::Client::ClientWrappedMessage::kIoBoardInit:
			so_5::send<io_board::init_interface>(_gpioBox);
			break;
		case LabNetProt::Client::ClientWrappedMessage::kIoBoardInitDigitalIn:
			{
				auto& init_in = mes->io_board_init_digital_in();
				so_5::send<io_board::init_digital_in>(_gpioBox, init_in.pin(), static_cast<io_board::resistor>(init_in.resistor_state()), init_in.is_inverted());
			}
			break;
		case LabNetProt::Client::ClientWrappedMessage::kIoBoardInitDigitalOut:
			{
				auto& init_out = mes->io_board_init_digital_out();
				so_5::send<io_board::init_digital_out>(_gpioBox, init_out.pin(), init_out.is_inverted());
			}
			break;
		case LabNetProt::Client::ClientWrappedMessage::kRfidBoardInit:
			{
				auto& init_sam = mes->rfid_board_init();
				so_5::send<rfid_board::init_interface>(_rfidBox, init_sam.antenna_phase1(), init_sam.antenna_phase2(), init_sam.phase_duration(), init_sam.inverted());
			}
			break;
		case LabNetProt::Client::ClientWrappedMessage::kRfidBoardSetPhaseMatrix:
			{
				auto& set_phase = mes->rfid_board_set_phase_matrix();
				so_5::send<rfid_board::set_phase_matrix>(_rfidBox, set_phase.antenna_phase1(), set_phase.antenna_phase2(), set_phase.phase_duration());
			}
			break;
		case LabNetProt::Client::ClientWrappedMessage::kRfidBoardSetSignalInversion:
			{
				auto& set_inv = mes->rfid_board_set_signal_inversion();
				so_5::send<rfid_board::set_signal_inversion>(_rfidBox, set_inv.inverted());
			}
			break;
		case LabNetProt::Client::ClientWrappedMessage::kUartInit:
			{
				_logger->writeInfoEntry("uart init mes");
				auto& uart_init = mes->uart_init();
				so_5::send<uart::messages::init_port>(_uartBox, static_cast<Interface::Interfaces>(uart_init.port()), uart_init.baud());
			}
			break;
		case LabNetProt::Client::ClientWrappedMessage::kUartWriteData:
			{
				auto& uart_write = mes->uart_write_data();
				std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>();
				data->resize(uart_write.data().size());
				for (int l = 0; l < uart_write.data().size(); l++)
					data->push_back(uart_write.data()[l]);
					
				so_5::send<StreamMessages::send_data_to_port>(_uartBox, static_cast<Interface::Interfaces>(uart_write.port()), 0, data);
			}
			break;
		case LabNetProt::Client::ClientWrappedMessage::kDigitalOutSet:
			{
				auto set = mes->digital_out_set();
				
				so_5::send<LabNetProt::Client::DigitalOutSet>(_digOutBox, set);
			}
			break;
		case LabNetProt::Client::ClientWrappedMessage::kDigitalOutPulse:
			{
				_logger->writeInfoEntry("digital out pulse mes");
				
				auto setPulse = mes->digital_out_pulse();
				so_5::send<LabNetProt::Client::DigitalOutPulse>(_digOutBox, setPulse);
			}
			break;
		case LabNetProt::Client::ClientWrappedMessage::kStartDigitalOutLoop:
			{
				_logger->writeInfoEntry("start digital out loop mes");
				auto setLoop = mes->start_digital_out_loop();
				
				so_5::send<LabNetProt::Client::StartDigitalOutLoop>(_digOutBox, setLoop);
			}
			break;
		case LabNetProt::Client::ClientWrappedMessage::kStopDigitalOutLoop:
			{
				_logger->writeInfoEntry("stop digital out loop mes");
				auto stopLoop = mes->stop_digital_out_loop();
				
				so_5::send<LabNetProt::Client::StopDigitalOutLoop>(_digOutBox, stopLoop);
			}
			break;
		case LabNetProt::Client::ClientWrappedMessage::kResetRequest:
			{
				so_5::send<Interface::reset_interface>(_gpioBox);
				so_5::send<Interface::reset_interface>(_interfaceManager);
				so_5::send<Interface::reset_interface>(_uartBox);
				
				std::shared_ptr<LabNetProt::Server::ServerWrappedMessage> swm = std::make_shared<LabNetProt::Server::ServerWrappedMessage>();
				LabNetProt::Server::LabNetResetReply* reply = new LabNetProt::Server::LabNetResetReply();
				reply->set_is_reset(true);
				swm->set_allocated_reset(reply);
				
				_connection->send_message(swm);
			}
			break;
		case LabNetProt::Client::ClientWrappedMessage::kIdRequest:
			{
				_logger->writeInfoEntry("id mes");
				
				std::shared_ptr<LabNetProt::Server::ServerWrappedMessage> swm = std::make_shared<LabNetProt::Server::ServerWrappedMessage>();
				LabNetProt::Server::LabNetIdReply* id = new LabNetProt::Server::LabNetIdReply();
				id->set_id("LabNet");
				id->set_major_version(1);
				id->set_minor_version(0);
				swm->set_allocated_id(id);
				_connection->send_message(swm);
			}
			break;
		case LabNetProt::Client::ClientWrappedMessage::CLIENT_MESSAGE_NOT_SET:
			break;
		}
	})
	.event([this](mhood_t<Interface::interface_init_result> mes) {
		std::shared_ptr<LabNetProt::Server::ServerWrappedMessage> swm = std::make_shared<LabNetProt::Server::ServerWrappedMessage>();
		LabNetProt::Server::InterfaceInitResult* init = new LabNetProt::Server::InterfaceInitResult();
		init->set_interface(static_cast<LabNetProt::Interfaces>(mes->interface));
		init->set_is_succeed(mes->is_succeed);
		swm->set_allocated_interface_init_result(init);
		_connection->send_message(swm);
	})
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
	;
}