#include "LabNetMainActor.h"
#include "LabNetMainActorMessages.h"
#include "Network/LabNetClientMessages.pb.h"
#include "Interface/GPIO/Messages.h"
#include "Interface/InterfaceMessages.h"
#include "Interface/SAM32/SamMessages.h"
#include "Interface/UART/SerialPortMessages.h"

using namespace LabNet;

LabNetMainActor::LabNetMainActor(context_t ctx, Logger logger, so_5::mbox_t gpioBox, so_5::mbox_t sam32Box, so_5::mbox_t uartBox)
	: so_5::agent_t(ctx)
	, _logger(logger)
	, _gpioBox(gpioBox)
	, _sam32Box(sam32Box)
	, _uartBox(uartBox)
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
			so_5::send<Interface::continue_interface>(_sam32Box);
			so_5::send<Interface::continue_interface>(_uartBox);
			
			this >>= connected;
		});
	
	connected
		.event([this](mhood_t<Disconnected>) {
		_connection.reset();
			
		so_5::send<Interface::pause_interface>(_gpioBox);
		so_5::send<Interface::pause_interface>(_sam32Box);
		so_5::send<Interface::pause_interface>(_uartBox);
			
		this >>= wait_for_connection;
	})
	.event([this](std::shared_ptr<LabNet::Messages::Client::ClientWrappedMessage> mes) {
		switch (mes->client_message_case())
		{
		case LabNet::Messages::Client::ClientWrappedMessage::kGpioInit:
			so_5::send<GPIO::init_interface>(_gpioBox);
			break;
		case LabNet::Messages::Client::ClientWrappedMessage::kGpioInitDigitalIn:
			{
				auto& init_in = mes->gpio_init_digital_in();
				so_5::send<GPIO::init_digital_in>(_gpioBox, init_in.pin(), static_cast<GPIO::resistor>(init_in.resistor_state()), init_in.is_inverted());
			}
			break;
		case LabNet::Messages::Client::ClientWrappedMessage::kGpioInitDigitalOut:
			{
				auto& init_out = mes->gpio_init_digital_out();
				so_5::send<GPIO::init_digital_out>(_gpioBox, init_out.pin(), init_out.is_inverted());
			}
			break;
		case LabNet::Messages::Client::ClientWrappedMessage::kSam32Init:
			{
				_logger->writeInfoEntry("sam32 init mes");
				auto& init_sam = mes->sam32_init();
				so_5::send<SAM::init_interface>(_sam32Box, init_sam.antenna_phase1(), init_sam.antenna_phase2(), init_sam.phase_duration(), init_sam.inverted());
			}
			break;
		case LabNet::Messages::Client::ClientWrappedMessage::kSam32SetPhaseMatrix:
			{
				_logger->writeInfoEntry("sam32 set phase matrix mes");
				auto& set_phase = mes->sam32_set_phase_matrix();
				so_5::send<SAM::set_phase_matrix>(_sam32Box, set_phase.antenna_phase1(), set_phase.antenna_phase2(), set_phase.phase_duration());
			}
			break;
		case LabNet::Messages::Client::ClientWrappedMessage::kSam32SetSignalInversion:
			{
				_logger->writeInfoEntry("sam32 set signal inversion mes");
				auto& set_inv = mes->sam32_set_signal_inversion();
				so_5::send<SAM::set_signal_inversion>(_sam32Box, set_inv.inverted());
			}
			break;
		case LabNet::Messages::Client::ClientWrappedMessage::kUartInit:
			{
				auto& uart_init = mes->uart_init();
				so_5::send<uart::messages::init_port>(_uartBox, uart_init.port() - 100, uart_init.baud());
			}
			break;
		case LabNet::Messages::Client::ClientWrappedMessage::kUartWriteData:
			{
				auto& uart_write = mes->uart_write_data();
				std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>();
				data->resize(uart_write.data().size());
				for (int l = 0; l < uart_write.data().size(); l++)
					data->push_back(uart_write.data()[l]);
					
				so_5::send<uart::messages::send_data_to_port>(_uartBox, uart_write.port() - 100, data);
			}
			break;
		case LabNet::Messages::Client::ClientWrappedMessage::kDigitalOutSet:
			{
				auto set = mes->digital_out_set();
				if (set.state())
				{
					_logger->writeInfoEntry("digital out on set mes");
				}
				else
					_logger->writeInfoEntry("digital out off set mes");
				
				so_5::send<GPIO::set_digital_out>(_gpioBox, set.id().pin(), set.state(), so_direct_mbox());
			}
			break;
		case LabNet::Messages::Client::ClientWrappedMessage::kDigitalOutPulse:
			_logger->writeInfoEntry("digital out pulse mes");
			break;
		case LabNet::Messages::Client::ClientWrappedMessage::kDigitalOutLoop:
			_logger->writeInfoEntry("digital out loop mes");
			break;
		case LabNet::Messages::Client::ClientWrappedMessage::kReset:
			{
				so_5::send<Interface::reset_interface>(_gpioBox);
				so_5::send<Interface::reset_interface>(_interfaceManager);
				
				std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Messages::Server::ServerWrappedMessage>();
				LabNet::Messages::Server::LabNetResetReply* reply = new LabNet::Messages::Server::LabNetResetReply();
				reply->set_is_reset(true);
				swm->set_allocated_reset(reply);
				
				_connection->send_message(swm);
			}
			break;
		case LabNet::Messages::Client::ClientWrappedMessage::kId:
			{
				_logger->writeInfoEntry("id mes");
				
				std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Messages::Server::ServerWrappedMessage>();
				LabNet::Messages::Server::LabNetIdReply* id = new LabNet::Messages::Server::LabNetIdReply();
				id->set_id("LabNet");
				id->set_major_version(1);
				id->set_minor_version(0);
				swm->set_allocated_id(id);
				_connection->send_message(swm);
			}
			break;
		case LabNet::Messages::Client::ClientWrappedMessage::CLIENT_MESSAGE_NOT_SET:
			break;
		}
	})
	.event([this](mhood_t<GPIO::interface_init_result> mes) {
		std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Messages::Server::ServerWrappedMessage>();
		LabNet::Messages::Server::InterfaceInitResult* init = new LabNet::Messages::Server::InterfaceInitResult();
		init->set_interface(LabNet::Messages::Server::Interfaces::GPIO_TOP_PLANE);
		init->set_is_succeed(mes->is_succeed);
		swm->set_allocated_interface_init_result(init);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<GPIO::return_digital_in_state> mes) {
		std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Messages::Server::ServerWrappedMessage>();
		LabNet::Messages::Server::DigitalInState* state = new LabNet::Messages::Server::DigitalInState();
		LabNet::Messages::Server::PinId *id = new LabNet::Messages::Server::PinId();
		id->set_interface(LabNet::Messages::Server::Interfaces::GPIO_TOP_PLANE);
		id->set_pin(mes->pin);
		state->set_allocated_pin(id);
		state->set_state(mes->state);
			
		swm->set_allocated_digital_in_state(state);
		_connection->send_message(swm);
		
		_logger->writeInfoEntry("return digIn state");
	})
	.event([this](mhood_t<GPIO::digital_in_init_result> mes)
	{
		std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Messages::Server::ServerWrappedMessage>();
		LabNet::Messages::Server::DigitalInInitResult* res = new LabNet::Messages::Server::DigitalInInitResult();
		res->set_interface(LabNet::Messages::Server::Interfaces::GPIO_TOP_PLANE);
		res->set_pin(mes->pin);
		res->set_is_succeed(mes->is_succeed);
			
		swm->set_allocated_digital_in_init_result(res);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<GPIO::digital_out_init_result> mes)
	{
		std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Messages::Server::ServerWrappedMessage>();
		LabNet::Messages::Server::DigitalOutInitResult* res = new LabNet::Messages::Server::DigitalOutInitResult();
		res->set_interface(LabNet::Messages::Server::Interfaces::GPIO_TOP_PLANE);
		res->set_pin(mes->pin);
		res->set_is_succeed(mes->is_succeed);
			
		swm->set_allocated_digital_out_init_result(res);
		_connection->send_message(swm);
		
		
	})
	.event([this](mhood_t<GPIO::return_digital_out_state> mes)
	{
		std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Messages::Server::ServerWrappedMessage>();
		LabNet::Messages::Server::DigitalOutState* state = new LabNet::Messages::Server::DigitalOutState();
		LabNet::Messages::Server::PinId *id = new LabNet::Messages::Server::PinId();
		id->set_interface(LabNet::Messages::Server::Interfaces::GPIO_TOP_PLANE);
		id->set_pin(mes->pin);
		state->set_allocated_pin(id);
		state->set_state(mes->state);
		
		swm->set_allocated_digital_out_state(state);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<SAM::interface_init_result> mes)
	{
		std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Messages::Server::ServerWrappedMessage>();
		LabNet::Messages::Server::InterfaceInitResult* init = new LabNet::Messages::Server::InterfaceInitResult();
		init->set_interface(LabNet::Messages::Server::Interfaces::SAM32);
		init->set_is_succeed(mes->is_succeed);
		swm->set_allocated_interface_init_result(init);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<SAM::new_data> mes)
	{
		std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Messages::Server::ServerWrappedMessage>();
		LabNet::Messages::Server::NewByteData* data = new LabNet::Messages::Server::NewByteData();
		LabNet::Messages::Server::PinId *id = new LabNet::Messages::Server::PinId();
		id->set_interface(LabNet::Messages::Server::Interfaces::GPIO_TOP_PLANE);
		id->set_pin(mes->port_id);
		data->set_allocated_pin(id);
		data->set_data(mes->data->data(), mes->data->size());
		
		swm->set_allocated_new_byte_data(data);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<uart::messages::init_port_result> mes)
	{
		std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Messages::Server::ServerWrappedMessage>();
		LabNet::Messages::Server::InterfaceInitResult* init = new LabNet::Messages::Server::InterfaceInitResult();
		if (mes->port_id == 0)
			init->set_interface(LabNet::Messages::Server::Interfaces::UART0);
		else if (mes->port_id == 1)
			init->set_interface(LabNet::Messages::Server::Interfaces::UART1);
		else if (mes->port_id == 2)
			init->set_interface(LabNet::Messages::Server::Interfaces::UART2);
		else if (mes->port_id == 3)
			init->set_interface(LabNet::Messages::Server::Interfaces::UART3);
		else if (mes->port_id == 4)
			init->set_interface(LabNet::Messages::Server::Interfaces::UART4);
		else
			init->set_interface(LabNet::Messages::Server::Interfaces::NONE);
		init->set_is_succeed(mes->is_succeed);
		swm->set_allocated_interface_init_result(init);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<uart::messages::port_unexpected_closed> mes)
	{
		std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Messages::Server::ServerWrappedMessage>();
		LabNet::Messages::Server::InterfaceLost* lost = new LabNet::Messages::Server::InterfaceLost();
		if (mes->port_id == 0)
			lost->set_interface(LabNet::Messages::Server::Interfaces::UART0);
		else if (mes->port_id == 1)
			lost->set_interface(LabNet::Messages::Server::Interfaces::UART1);
		else if (mes->port_id == 2)
			lost->set_interface(LabNet::Messages::Server::Interfaces::UART2);
		else if (mes->port_id == 3)
			lost->set_interface(LabNet::Messages::Server::Interfaces::UART3);
		else if (mes->port_id == 4)
			lost->set_interface(LabNet::Messages::Server::Interfaces::UART4);
		else
			lost->set_interface(LabNet::Messages::Server::Interfaces::NONE);
		
		swm->set_allocated_interface_lost(lost);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<uart::messages::port_reconnected> mes)
	{
		std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Messages::Server::ServerWrappedMessage>();
		LabNet::Messages::Server::InterfaceReconnected* lost = new LabNet::Messages::Server::InterfaceReconnected();
		if (mes->port_id == 0)
			lost->set_interface(LabNet::Messages::Server::Interfaces::UART0);
		else if (mes->port_id == 1)
			lost->set_interface(LabNet::Messages::Server::Interfaces::UART1);
		else if (mes->port_id == 2)
			lost->set_interface(LabNet::Messages::Server::Interfaces::UART2);
		else if (mes->port_id == 3)
			lost->set_interface(LabNet::Messages::Server::Interfaces::UART3);
		else if (mes->port_id == 4)
			lost->set_interface(LabNet::Messages::Server::Interfaces::UART4);
		else
			lost->set_interface(LabNet::Messages::Server::Interfaces::NONE);
		
		swm->set_allocated_interface_reconnected(lost);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<uart::messages::new_data_from_port> mes)
	{
		std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Messages::Server::ServerWrappedMessage>();
		LabNet::Messages::Server::NewByteData* data = new LabNet::Messages::Server::NewByteData();
		
		data->set_data(mes->data->data(), mes->data->size());
		
		LabNet::Messages::Server::PinId *id = new LabNet::Messages::Server::PinId();
		id->set_pin(0);
		if (mes->port_id == 0)
			id->set_interface(LabNet::Messages::Server::Interfaces::UART0);
		else if (mes->port_id == 1)
			id->set_interface(LabNet::Messages::Server::Interfaces::UART1);
		else if (mes->port_id == 2)
			id->set_interface(LabNet::Messages::Server::Interfaces::UART2);
		else if (mes->port_id == 3)
			id->set_interface(LabNet::Messages::Server::Interfaces::UART3);
		else if (mes->port_id == 4)
			id->set_interface(LabNet::Messages::Server::Interfaces::UART4);
		else
			id->set_interface(LabNet::Messages::Server::Interfaces::NONE);
		data->set_allocated_pin(id);
		
		swm->set_allocated_new_byte_data(data);
		_connection->send_message(swm);
	})
	.event([this](mhood_t<uart::messages::send_data_complete> mes)
	{
		std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Messages::Server::ServerWrappedMessage>();
		LabNet::Messages::Server::DataWriteComplete* write = new LabNet::Messages::Server::DataWriteComplete();
		
		LabNet::Messages::Server::PinId *id = new LabNet::Messages::Server::PinId();
		id->set_pin(0);
		if (mes->port_id == 0)
			id->set_interface(LabNet::Messages::Server::Interfaces::UART0);
		else if (mes->port_id == 1)
			id->set_interface(LabNet::Messages::Server::Interfaces::UART1);
		else if (mes->port_id == 2)
			id->set_interface(LabNet::Messages::Server::Interfaces::UART2);
		else if (mes->port_id == 3)
			id->set_interface(LabNet::Messages::Server::Interfaces::UART3);
		else if (mes->port_id == 4)
			id->set_interface(LabNet::Messages::Server::Interfaces::UART4);
		else
			id->set_interface(LabNet::Messages::Server::Interfaces::NONE);
		write->set_allocated_pin(id);
		
		swm->set_allocated_data_write_complete(write);
		_connection->send_message(swm);
	})
	;
}