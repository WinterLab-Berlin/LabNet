#include <iostream>
#include "client.h"
#include <so_5/all.hpp>
#include "id_test.h"
#include "set_dig_out_test.h"
#include "set_and_read_dig_out_test.h"


int main(int argc, char* argv[])
{
    so_5::wrapped_env_t sobj;
    so_5::mbox_t first_test_box;
    so_5::mbox_t second_test_box;
    so_5::mbox_t third_test_box;

    std::shared_ptr<Client> client = std::make_shared<Client>("192.168.137.101", 8080);

    sobj.environment().introduce_coop(
        [&](so_5::coop_t& coop) {
            first_test_box = coop.environment().create_mbox();
            second_test_box = coop.environment().create_mbox();
            third_test_box = coop.environment().create_mbox();


            auto act1 = coop.make_agent<id_test>(first_test_box, second_test_box, client, 100000);
            auto act2 = coop.make_agent<set_dig_out_test>(second_test_box, third_test_box, client, 100000);
            auto act3 = coop.make_agent<set_and_read_dig_out_test>(third_test_box, nullptr, client, 100000);
        });

    client->SetRecvBox(first_test_box);
    client->Start();
    
    return 0;
}