#pragma once

#include <array>
#include <memory>
#include <boost/asio.hpp>
#include <vector>
#include <LoggingFacility.h>
#include "LabNetServerMessages.pb.h"
#include "LabNetClientMessages.pb.h"

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
	
	void refuse_conection();
	
	void send_message(std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> mes);

private:
	void start_read_header();
	void start_read_body(unsigned msg_len);
	bool pack_msg(std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> msg, std::vector<char> &data_buffer);
	void encode_header(unsigned size, std::vector<char> &data_buffer);
	unsigned decode_header();
	void handle_request();

	// header size for packed messages
	const unsigned HEADER_SIZE = 2;
	// max size of packed messages
	const unsigned MAX_MSG_SIZE = 0xFFFF;
	
	Logger m_logger;
	boost::asio::ip::tcp::socket m_socket;
	ConnectionManager& m_connection_manager;
	std::vector<char> m_readBuffer;
	bool m_globBufferEmpty;
};