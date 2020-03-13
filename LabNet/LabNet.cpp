#define BOOST_BIND_NO_PLACEHOLDERS

#include <LoggerFactory.h>
#include "Network/Server.h"
#include <so_5/all.hpp>
#include "Interface/UART/SerialPortsManager.h"
#include "Interface/UART/SerialPortMessages.h"

class test final : public so_5::agent_t
{
public:
	test(context_t ctx, so_5::mbox_t ser_port, Logger logger)
		: so_5::agent_t{ ctx }
		, m_ser_port(ser_port)
		, m_logger(logger)
	{
		
	}

	void so_define_agent() override
	{
		so_default_state()
			.event(&test::init_port_error_event)
			.event(&test::init_init_port_success_event);
	}

	void so_evt_start() override
	{
		so_5::send_delayed<init_port>(m_ser_port, std::chrono::microseconds(100), 1, 9600, so_direct_mbox());
	}
	
private:
	void init_port_error_event(const init_port_error& ev)
	{
		m_logger->writeInfoEntry("port init error from test");
	}
	void init_init_port_success_event(const init_port_success& ev)
	{
		m_logger->writeInfoEntry("port init success from test");
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
			auto serPort = coop.make_agent< SerialPortsManager >(logger);
			m = serPort->so_direct_mbox();
			
			env.register_agent_as_coop(env.make_agent<test>(m, logger));
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