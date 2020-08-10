#include "LabNetMainActor.h"
#include "LabNetMainActorMessages.h"
#include "Network/ProtocolAll.h"
#include "Interface/GPIO/Messages.h"
#include "Interface/InterfaceMessages.h"
#include "Interface/RFID/RfidMessages.h"
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
	.event([this](std::shared_ptr<LabNet::Client::ClientWrappedMessage> mes) {
		switch (mes->client_message_case())
		{
		case LabNet::Client::ClientWrappedMessage::kGpioInit:
			so_5::send<GPIO::init_interface>(_gpioBox);
			break;
		case LabNet::Client::ClientWrappedMessage::kGpioInitDigitalIn:
			{
				auto& init_in = mes->gpio_init_digital_in();
				so_5::send<GPIO::init_digital_in>(_gpioBox, init_in.pin(), static_cast<GPIO::resistor>(init_in.resistor_state()), init_in.is_inverted());
			}
			break;
		case LabNet::Client::ClientWrappedMessage::kGpioInitDigitalOut:
			{
				auto& init_out = mes->gpio_init_digital_out();
				so_5::send<GPIO::init_digital_out>(_gpioBox, init_out.pin(), init_out.is_inverted());
			}
			break;
		case LabNet::Client::ClientWrappedMessage::kRfidInit:
			{
				_logger->writeInfoEntry("rfid init mes");
				auto& init_sam = mes->rfid_init();
				so_5::send<RFID::init_interface>(_rfidBox, init_sam.antenna_phase1(), init_sam.antenna_phase2(), init_sam.phase_duration(), init_sam.inverted());
			}
			break;
		case LabNet::Client::ClientWrappedMessage::kRfidSetPhaseMatrix:
			{
				_logger->writeInfoEntry("rfid set phase matrix mes");
				auto& set_phase = mes->rfid_set_phase_matrix();
				so_5::send<RFID::set_phase_matrix>(_rfidBox, set_phase.antenna_phase1(), set_phase.antenna_phase2(), set_phase.phase_duration());
			}
			break;
		case LabNet::Client::ClientWrappedMessage::kRfidSetSignalInversion:
			{
				_logger->writeInfoEntry("rfid set signal inversion mes");
				auto& set_inv = mes->rfid_set_signal_inversion();
				so_5::send<RFID::set_signal_inversion>(_rfidBox, set_inv.inverted());
			}
			break;
		case LabNet::Client::ClientWrappedMessage::kUartInit:
			{
				_logger->writeInfoEntry("uart init mes");
				auto& uart_init = mes->uart_init();
				so_5::send<uart::messages::init_port>(_uartBox, static_cast<Interface::Interfaces>(uart_init.port()), uart_init.baud());
			}
			break;
		case LabNet::Client::ClientWrappedMessage::kUartWriteData:
			{
				auto& uart_write = mes->uart_write_data();
				std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>();
				data->resize(uart_write.data().size());
				for (int l = 0; l < uart_write.data().size(); l++)
					data->push_back(uart_write.data()[l]);
					
				so_5::send<StreamMessages::send_data_to_port>(_uartBox, static_cast<Interface::Interfaces>(uart_write.port()), 0, data);
			}
			break;
		case LabNet::Client::ClientWrappedMessage::kDigitalOutSet:
			{
				auto set = mes->digital_out_set();
				
				so_5::send<LabNet::Client::DigitalOutSet>(_digOutBox, set);
			}
			break;
		case LabNet::Client::ClientWrappedMessage::kDigitalOutPulse:
			{
				_logger->writeInfoEntry("digital out pulse mes");
				
				auto setPulse = mes->digital_out_pulse();
				so_5::send<LabNet::Client::DigitalOutPulse>(_digOutBox, setPulse);
			}
			break;
		case LabNet::Client::ClientWrappedMessage::kStartDigitalOutLoop:
			{
				_logger->writeInfoEntry("start digital out loop mes");
				auto setLoop = mes->start_digital_out_loop();
				
				so_5::send<LabNet::Client::StartDigitalOutLoop>(_digOutBox, setLoop);
			}
			break;
		case LabNet::Client::ClientWrappedMessage::kStopDigitalOutLoop:
			{
				_logger->writeInfoEntry("stop digital out loop mes");
				auto stopLoop = mes->stop_digital_out_loop();
				
				so_5::send<LabNet::Client::StopDigitalOutLoop>(_digOutBox, stopLoop);
			}
			break;
		case LabNet::Client::ClientWrappedMessage::kReset:
			{
				so_5::send<Interface::reset_interface>(_gpioBox);
				so_5::send<Interface::reset_interface>(_interfaceManager);
				so_5::send<Interface::reset_interface>(_uartBox);
				
				std::shared_ptr<LabNet::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Server::ServerWrappedMessage>();
				LabNet::Server::LabNetResetReply* reply = new LabNet::Server::LabNetResetReply();
				reply->set_is_reset(true);
				swm->set_allocated_reset(reply);
				
				_connection->send_message(swm);
			}
			break;
		case LabNet::Client::ClientWrappedMessage::kId:
			{
				_logger->writeInfoEntry("id mes");
				
				std::shared_ptr<LabNet::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Server::ServerWrappedMessage>();
				LabNet::Server::LabNetIdReply* id = new LabNet::Server::LabNetIdReply();
				id->set_id("LabNet");
				id->set_major_version(1);
				id->set_minor_version(0);
				swm->set_allocated_id(id);
				_connection->send_message(swm);
			}
			break;
		case LabNet::Client::ClientWrappedMessage::CLIENT_MESSAGE_NOT_SET:
			break;
		}
	})
	.event([this](mhood_t<Interface::interface_init_result> mes) {
		std::shared_ptr<LabNet::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Server::ServerWrappedMessage>();
		LabNet::Server::InterfaceInitResult* init = new LabNet::Server::InterfaceInitResult();
		init->set_interface(static_cast<LabNet::Interfaces>(mes->interface));
		init->set_is_succeed(mes->is_succeed);
		swm->set_allocated_interface_init_result(init);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<DigitalMessages::digital_in_init_result> mes)
	{
		std::shared_ptr<LabNet::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Server::ServerWrappedMessage>();
		LabNet::Server::DigitalInInitResult* res = new LabNet::Server::DigitalInInitResult();
		res->set_interface(static_cast<LabNet::Interfaces>(mes->interface));
		res->set_pin(mes->pin);
		res->set_is_succeed(mes->is_succeed);
			
		swm->set_allocated_digital_in_init_result(res);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<DigitalMessages::return_digital_in_state> mes) {
		std::shared_ptr<LabNet::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Server::ServerWrappedMessage>();
		LabNet::Server::DigitalInState* state = new LabNet::Server::DigitalInState();
		LabNet::PinId *id = new LabNet::PinId();
		id->set_interface(static_cast<LabNet::Interfaces>(mes->interface));
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
		std::shared_ptr<LabNet::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Server::ServerWrappedMessage>();
		LabNet::Server::DigitalOutInitResult* res = new LabNet::Server::DigitalOutInitResult();
		res->set_interface(static_cast<LabNet::Interfaces>(mes->interface));
		res->set_pin(mes->pin);
		res->set_is_succeed(mes->is_succeed);
			
		swm->set_allocated_digital_out_init_result(res);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<DigitalMessages::return_digital_out_state> mes)
	{
		std::shared_ptr<LabNet::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Server::ServerWrappedMessage>();
		LabNet::Server::DigitalOutState* state = new LabNet::Server::DigitalOutState();
		LabNet::PinId *id = new LabNet::PinId();
		id->set_interface(static_cast<LabNet::Interfaces>(mes->interface));
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
		std::shared_ptr<LabNet::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Server::ServerWrappedMessage>();
		LabNet::Server::InterfaceLost* lost = new LabNet::Server::InterfaceLost();
		lost->set_interface(static_cast<LabNet::Interfaces>(mes->interface));
		
		swm->set_allocated_interface_lost(lost);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<Interface::interface_reconnected> mes)
	{
		std::shared_ptr<LabNet::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Server::ServerWrappedMessage>();
		LabNet::Server::InterfaceReconnected* lost = new LabNet::Server::InterfaceReconnected();
		lost->set_interface(static_cast<LabNet::Interfaces>(mes->interface));
		
		swm->set_allocated_interface_reconnected(lost);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<StreamMessages::new_data_from_port> mes)
	{
		std::shared_ptr<LabNet::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Server::ServerWrappedMessage>();
		LabNet::Server::NewByteData* data = new LabNet::Server::NewByteData();
		
		data->set_data(mes->data->data(), mes->data->size());
		
		LabNet::PinId *id = new LabNet::PinId();
		id->set_pin(mes->pin);
		id->set_interface(static_cast<LabNet::Interfaces>(mes->interface));
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
		std::shared_ptr<LabNet::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Server::ServerWrappedMessage>();
		LabNet::Server::DataWriteComplete* write = new LabNet::Server::DataWriteComplete();
		
		LabNet::PinId *id = new LabNet::PinId();
		id->set_pin(0);
		id->set_interface(static_cast<LabNet::Interfaces>(mes->interface));
		write->set_allocated_pin(id);
		
		swm->set_allocated_data_write_complete(write);
		_connection->send_message(swm);
	})
	;
}