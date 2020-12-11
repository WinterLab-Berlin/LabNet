#pragma once

#define BOOST_BIND_NO_PLACEHOLDERS

#include <set>
#include <memory>
#include <boost/signals2.hpp>
#include <logging_facility.h>
#include <so_5/all.hpp>
#include "connection.h"
#include "protocol_all.h"

namespace LabNet::network
{
    class ConnectionManager
    {
    public:
        ConnectionManager(const ConnectionManager&) = delete;
        ConnectionManager& operator=(const ConnectionManager&) = delete;

        /// Construct a connection manager.
        ConnectionManager(log::Logger logger, so_5::mbox_t server_in_box);

        /// Add the specified connection to the manager and start it.
        void Start(std::shared_ptr<Connection> c);

        /// Stop the specified connection.
        void Stop(std::shared_ptr<Connection> c);

        /// Stop all connections.
        void StopAll();

        so_5::mbox_t GetServerInBox()
        {
            return server_in_box_;
        };

    private:
        log::Logger logger_;
        std::shared_ptr<Connection> connection_;
        std::set<std::shared_ptr<Connection>> connections_;
        so_5::mbox_t server_in_box_;
    };
};
