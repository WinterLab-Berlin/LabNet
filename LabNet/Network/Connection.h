#pragma once

#include <array>
#include <memory>
#include <boost/asio.hpp>
#include <vector>
#include <LoggingFacility.h>

class ConnectionManager;

class Connection
	: public std::enable_shared_from_this<Connection>
{
public:
	Connection(const Connection&) = delete;
	Connection& operator=(const Connection&) = delete;

	/// Construct a connection with the given socket.
	explicit Connection(Logger logger, boost::asio::ip::tcp::socket socket, ConnectionManager& connection_manager);

	/// Start the first asynchronous operation for the connection.
	void start();

	/// Stop all asynchronous operations associated with the connection.
	void stop();
	
	void refuse_conection(std::string message);
	
	void send_message(std::shared_ptr<std::vector<char>> mes);

private:
	void do_read();
	void do_write(std::vector<char> buffer);

	Logger m_logger;
	boost::asio::ip::tcp::socket m_socket;
	ConnectionManager& m_connection_manager;
	std::array<char, 8192> m_buffer;

};