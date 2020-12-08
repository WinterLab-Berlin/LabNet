#define BOOST_BIND_NO_PLACEHOLDERS

#include "DigitalOut/digital_out_helper.h"
#include "Interface/manage_interfaces.h"
#include "Interface/gpio_wiring/gpio_manager.h"
#include "Interface/io_board/board_actor.h"
#include "Interface/rfid_board/rfid_main_actor.h"
#include "Interface/uart/serial_ports_manager.h"
#include "resources/resources_actor.h"
#include "helper/reset_helper.h"
#include "network/server_actor.h"
#include "network/Server.h"
#include <LoggerFactory.h>
#include <so_5/all.hpp>
#include <wiringPi.h>

int main(int argc, char* argv[])
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
    so_5::mbox_t server_in_box;

    namespace pool_disp = so_5::disp::thread_pool;

    // Start SO-part of the app.
    sobj.environment().introduce_coop(
        //pool_disp::make_dispatcher(sobj.environment()).binder(),
        [&](so_5::coop_t& coop) {
            server_in_box = coop.environment().create_mbox("server_in");

            auto act = coop.make_agent<LabNet::network::server_actor>(logger);

            coop.make_agent<LabNet::interface::ManageInterfaces>(logger);
            coop.make_agent<LabNet::digital_out::DigitalOutHelper>(logger);
            coop.make_agent<LabNet::helper::ResetHelper>(logger);
            coop.make_agent<LabNet::resources::resources_actor>(logger);
        });

    ConnectionManager connection_manager(logger, server_in_box);

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