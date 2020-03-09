#pragma once

#include <boost/asio.hpp>
#include <string>
#include "Connection.h"
#include "ConnectionManager.h"

class Server
{
public:
	Server(const Server&) = delete;
	Server& operator=(const Server&) = delete;

	/// Construct the server to listen on the specified port
	explicit Server(ConnectionManager& connection_manager, ushort port);

	/// Run the server's io_context loop.
	void run();

private:
	/// Perform an asynchronous accept operation.
	void do_accept();

	/// Wait for a request to stop the server.
	void do_await_stop();

	/// The io_context used to perform asynchronous operations.
	boost::asio::io_context m_io_context;

	/// The signal_set is used to register for process termination notifications.
	boost::asio::signal_set m_signals;

	/// Acceptor used to listen for incoming connections.
	boost::asio::ip::tcp::acceptor m_acceptor;

	/// The connection manager which owns all live connections.
	ConnectionManager& m_connection_manager;
};