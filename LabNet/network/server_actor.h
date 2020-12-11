#pragma once

#include "connection.h"
#include <logging_facility.h>
#include <google/protobuf/message.h>
#include <map>
#include <so_5/all.hpp>
#include <vector>

namespace LabNet::network
{
    class ServerActor final : public so_5::agent_t
    {
    public:
        ServerActor(context_t ctx, log::Logger logger);
        ~ServerActor();

    private:
        void so_define_agent() override;

        so_5::state_t wait_for_connection_state_ { this, "wait for connection_state" };
        so_5::state_t connected_state_ { this, "connected_state" };
        so_5::state_t reset_state_ { this, "reset_state" };
        so_5::state_t reset_no_connection_state_ { this, "reset_no_connection_state" };

        log::Logger logger_;
        std::shared_ptr<Connection> connection_;
        so_5::mbox_t server_in_box_;
        so_5::mbox_t server_out_box_;
    };
}