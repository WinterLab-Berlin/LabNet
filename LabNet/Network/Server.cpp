#include "Server.h"
#include <signal.h>
#include <memory>
#include <utility>
#include <boost/lexical_cast.hpp>

Server::Server(Logger logger, ConnectionManager& connection_manager, ushort port)
	: m_logger(logger)
	, m_io_context(1)
	, m_signals(m_io_context)
	, m_acceptor(m_io_context)
	, m_connection_manager(connection_manager)
{
	// Register to handle the signals that indicate when the server should exit.
	m_signals.add(SIGINT);
	m_signals.add(SIGTERM);
#if defined(SIGQUIT)
	m_signals.add(SIGQUIT);
#endif // defined(SIGQUIT)

	do_await_stop();

	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);

	m_acceptor.open(endpoint.protocol());
	m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	m_acceptor.bind(endpoint);
	m_acceptor.listen();

	
	m_logger->writeInfoEntry(std::string("server is now running on: ") + boost::lexical_cast<std::string>(endpoint));
	
	do_accept();
}

void Server::run()
{
	m_io_context.run();
}

void Server::do_accept()
{
	m_acceptor.async_accept(
	    [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
	{
		// Check whether the server was stopped by a signal before this
		// completion handler had a chance to run.
		if(!m_acceptor.is_open())
		{
			return;
		}

		if (!ec)
		{
			m_logger->writeInfoEntry(std::string("new connection from ") + boost::lexical_cast<std::string>(socket.remote_endpoint()));
			
			socket.set_option(boost::asio::socket_base::keep_alive(true));
			socket.set_option(boost::asio::socket_base::linger(true, 2));
			
			boost::asio::ip::tcp::no_delay option(true);
			socket.set_option(option);
			
			m_connection_manager.start(std::make_shared<Connection>(
			    m_logger, std::move(socket), m_connection_manager));
		}

		do_accept();
	});
}

void Server::do_await_stop()
{
	m_signals.async_wait(
	    [this](boost::system::error_code /*ec*/, int /*signo*/)
	{
		m_acceptor.close();
		m_connection_manager.stop_all();
		
		m_logger->writeInfoEntry("stop server");
	});
}
