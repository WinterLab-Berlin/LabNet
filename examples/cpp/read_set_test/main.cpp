#include <iostream>
#include "client.h"
#include <so_5/all.hpp>
#include "read_and_set_test.h"


int main(int argc, char* argv[])
{
    so_5::wrapped_env_t sobj;
    so_5::mbox_t test_box;

    std::shared_ptr<Client> client = std::make_shared<Client>("192.168.137.101", 8080);

    sobj.environment().introduce_coop(
        [&](so_5::coop_t& coop) {
            test_box = coop.environment().create_mbox();

            auto act2 = coop.make_agent<read_and_set_test>(test_box, client);
        });

    client->SetRecvBox(test_box);
    client->Start();
    
    return 0;
}