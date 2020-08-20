#define BOOST_BIND_NO_PLACEHOLDERS

#include <LoggerFactory.h>
#include <wiringPi.h>
#include <so_5/all.hpp>
#include "Network/Server.h"
#include "LabNetMainActor.h"
#include "Interface/rfid_board/RfidMainActor.h"
#include "Interface/ManageInterfaces.h"
#include "Interface/io_board/GPIOManager.h"
#include "Interface/UART/SerialPortsManager.h"
#include "DigitalOut/DigitalOutHelper.h"

int main(int argc, char *argv[])
{
	Logger logger = LoggerFactory::create();
	logger->writeInfoEntry("LabNet starting");
	
	int err = wiringPiSetup();
	if (err == -1)
	{
		logger->writeInfoEntry("wiringPi init failed");
		return 0;
	}
	else
	{
		logger->writeInfoEntry("wiringPi init success");
	}
	
	// SO Environment in a special wrapper object.
	// Environment will be started automatically.
	so_5::wrapped_env_t sobj;
	so_5::mbox_t labNetBox;
	
	// Start SO-part of the app.
	sobj.environment().introduce_coop([&](so_5::coop_t & coop) {
		so_5::mbox_t gpioBox = coop.environment().create_mbox("gpio");
		so_5::mbox_t rfidBox = coop.environment().create_mbox("rfid");
		so_5::mbox_t uartBox = coop.environment().create_mbox("uart");
		so_5::mbox_t digOutBox = coop.environment().create_mbox("digOut");
		
		auto act = coop.make_agent<LabNet::LabNetMainActor>(logger, gpioBox, rfidBox, uartBox, digOutBox);
		labNetBox = act->so_direct_mbox();
		
		coop.make_agent<Interface::ManageInterfaces>();
		coop.make_agent<io_board::GPIOManager>(gpioBox, labNetBox, logger);
		coop.make_agent<rfid_board::SamMainActor>(rfidBox, labNetBox, logger);
		coop.make_agent<uart::SerialPortsManager>(uartBox, labNetBox, logger);
		coop.make_agent<DigitalOut::DigitalOutHelper>(logger, digOutBox, labNetBox);
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