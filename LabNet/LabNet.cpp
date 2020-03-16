#define BOOST_BIND_NO_PLACEHOLDERS

#include <LoggerFactory.h>
#include "Network/Server.h"
#include <so_5/all.hpp>
#include "Interface/UART/SerialPortsManager.h"
#include "Interface/UART/SerialPortMessages.h"

struct move_to_first
{
};

struct move_to_second
{
};

class test final : public so_5::agent_t
{
public:
	test(context_t ctx, Logger logger)
		: so_5::agent_t{ ctx }
		, m_logger(logger)
	{
		
	}

	void so_define_agent() override
	{
		so_default_state()
			.event(&test::init_port_error_event)
			.event(&test::init_init_port_success_event)
			.event(&test::move_motor_to_first)
			.event(&test::move_motor_to_second);
	}

	void so_evt_start() override
	{
		so_5::introduce_child_coop( *this,
			[&](so_5::coop_t & coop) {
				auto serPort = coop.make_agent< SerialPortsManager >(so_direct_mbox(), m_logger);
				m_ser_port = serPort->so_direct_mbox();
			});
		
		so_5::send<init_port>(m_ser_port, 1, 57600);
	}
	
private:
	void init_port_error_event(const init_port_error& ev)
	{
		m_logger->writeInfoEntry("port init error from test");
	}
	void init_init_port_success_event(const init_port_success& ev)
	{
		m_logger->writeInfoEntry("port init success from test");
		
		so_5::send_delayed<move_to_first>(so_direct_mbox(), std::chrono::seconds(1));
	}
	void move_motor_to_first(const move_to_first& m)
	{
		m_logger->writeInfoEntry("move to first");
		
		std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>();
		data->push_back(0xFF);
		data->push_back(0xFF);
		data->push_back(0x02);
		data->push_back(0x05);
		data->push_back(0x03);
		data->push_back(0x1E);
		data->push_back(0x99);
		data->push_back(0x02);
		data->push_back(0x3C);
		
		so_5::send<send_data_to_port>(m_ser_port, 1, data);
		so_5::send_delayed<move_to_second>(so_direct_mbox(), std::chrono::seconds(3));
	}
	void move_motor_to_second(const move_to_second& m)
	{
		m_logger->writeInfoEntry("move to second");
		
		std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>();
		data->push_back(0xFF);
		data->push_back(0xFF);
		data->push_back(0x02);
		data->push_back(0x05);
		data->push_back(0x03);
		data->push_back(0x1E);
		data->push_back(0x33);
		data->push_back(0x01);
		data->push_back(0xA3);
		
		so_5::send<send_data_to_port>(m_ser_port, 1, data);
		
		so_5::send_delayed<move_to_first>(so_direct_mbox(), std::chrono::seconds(3));
	}
	
	so_5::mbox_t m_ser_port;
	Logger m_logger;
};

int main(int argc, char *argv[])
{
	Logger logger = LoggerFactory::create();
	logger->writeInfoEntry("LabNet starting");
	
	so_5::launch([&](so_5::environment_t & env) {
		so_5::mbox_t m;
		env.introduce_coop([&](so_5::coop_t & coop) {
			env.register_agent_as_coop(env.make_agent<test>(logger));
		});
	});

	
//	NetworkProxyActor proxy(logger);
//	ConnectionManager connection_manager(logger, proxy);
//	
//	try
//	{
//		Server server(logger, connection_manager, 8080);
//		server.run();
//	} 
//	catch (std::exception& e)
//	{
//		logger->writeFatalEntry(std::string("network server error ") + e.what());
//	}
	
	
	
	return 0;
}