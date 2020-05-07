#define BOOST_BIND_NO_PLACEHOLDERS

#include <LoggerFactory.h>
#include <wiringPi.h>
#include <so_5/all.hpp>
#include "Interface/SAM32/SamMainActor.h"
#include "Network/Server.h"
#include "LabNetMainActor.h"
#include "Interface/ManageInterfaces.h"
#include "Interface/GPIO/GPIOManager.h"


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
	
	// SO Environment in a special wrapper object.
	// Environment will be started automatically.
	so_5::wrapped_env_t sobj;
	so_5::mbox_t labNetBox;
	
	// Start SO-part of the app.
	sobj.environment().introduce_coop([&](so_5::coop_t & coop) {
		so_5::mbox_t gpioBox = coop.environment().create_mbox("gpio");
		so_5::mbox_t sam32Box = coop.environment().create_mbox("sam32");
		
		auto act = coop.make_agent<LabNet::LabNetMainActor>(logger, gpioBox, sam32Box);
		labNetBox = act->so_direct_mbox();
		
		coop.make_agent<Interface::ManageInterfaces>();
		coop.make_agent<GPIO::GPIOManager>(gpioBox, labNetBox, logger);
		coop.make_agent<SAM::SamMainActor>(sam32Box, labNetBox, logger);
	});
	
	ConnectionManager connection_manager(logger, labNetBox);
		
	try
	{
		Server server(logger, connection_manager, 8080);
		server.run();
	} 
	catch (std::exception& e)
	{
		logger->writeFatalEntry(std::string("network server error ") + e.what());
	}
	
	return 0;
}