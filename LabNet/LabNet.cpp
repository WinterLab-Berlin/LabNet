#define BOOST_BIND_NO_PLACEHOLDERS

#include <LoggerFactory.h>
#include <wiringPi.h>
#include <so_5/all.hpp>
#include "Interface/SAM32/SamMainActor.h"



int main(int argc, char *argv[])
{
	Logger logger = LoggerFactory::create();
	logger->writeInfoEntry("LabNet starting");
	
	int err = wiringPiSetup();
	if (err == -1)
	{
		logger->writeInfoEntry("GPIO init failed");
		return 0;
	}
	else
	{
		logger->writeInfoEntry("GPIO init success");
	}
	
	//	so_5::launch([&](so_5::environment_t & env) {
	//		so_5::mbox_t m;
	//		env.introduce_coop([&](so_5::coop_t & coop) {
	//			env.register_agent_as_coop(env.make_agent<test>(logger));
	//		});
	//	});

	
	
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