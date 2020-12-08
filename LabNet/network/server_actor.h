#pragma once

#include "Connection.h"
#include <LoggingFacility.h>
#include <google/protobuf/message.h>
#include <map>
#include <so_5/all.hpp>
#include <vector>

namespace LabNet::network
{
    class server_actor final : public so_5::agent_t
    {
    public:
        server_actor(context_t ctx, Logger logger);
        ~server_actor();

    private:
        void so_define_agent() override;

        so_5::state_t wait_for_connection_state { this, "wait for connection_state" };
        so_5::state_t connected_state { this, "connected_state" };
        so_5::state_t reset_state { this, "reset_state" };
        so_5::state_t reset_no_connection_state { this, "reset_no_connection_state" };

        Logger _logger;
        std::shared_ptr<Connection> _connection;
        so_5::mbox_t _server_in_box;
        so_5::mbox_t _server_out_box;
    };
}