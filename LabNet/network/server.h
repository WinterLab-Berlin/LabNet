#pragma once

#include <boost/asio.hpp>
#include <string>
#include <logging_facility.h>
#include "connection_manager.h"

namespace LabNet::network
{
    class Server
    {
    public:
        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;

        /// Construct the server to listen on the specified port
        explicit Server(log::Logger logger, ConnectionManager& connection_manager, ushort port);

        /// Run the server's io_context loop.
        void Run();

    private:
        /// Perform an asynchronous accept operation.
        void DoAccept();

        /// Wait for a request to stop the server.
        void DoAwaitStop();

        /// The io_context used to perform asynchronous operations.
        boost::asio::io_context io_context_;

        /// The signal_set is used to register for process termination notifications.
        boost::asio::signal_set signals_;

        /// Acceptor used to listen for incoming connections.
        boost::asio::ip::tcp::acceptor acceptor_;

        /// The connection manager which owns all live connections.
        ConnectionManager& connection_manager_;
        log::Logger logger_;
    };
};