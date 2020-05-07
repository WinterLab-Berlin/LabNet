#include <iostream>
#include "Connection.h"
#include "ConnectionManager.h"

Connection::Connection(Logger logger,
	boost::asio::ip::tcp::socket socket,
	ConnectionManager &connection_manager)
	: m_logger(logger)
	, m_socket(std::move(socket))
	, m_connection_manager(connection_manager)
{
}

void Connection::start()
{
	start_read_header();
}

void Connection::stop()
{
	m_logger->writeInfoEntry("connection stopped");
	
	m_socket.close();
}

void Connection::start_read_header()
{
	auto self(shared_from_this());
	
	m_readBuffer.resize(HEADER_SIZE);
	boost::asio::async_read(
		m_socket,
		boost::asio::buffer(m_readBuffer),
		[this, self](boost::system::error_code ec, std::size_t bytes_transferred)
		{
			if (!ec)
			{
				unsigned msg_len = decode_header();
				if (msg_len > 0)
				{
					start_read_body(msg_len);
				}
				else
				{
					start_read_header();
				}
			}
			else if (ec != boost::asio::error::operation_aborted)
			{
				m_connection_manager.stop(shared_from_this());
			}
		});
}

void Connection::start_read_body(unsigned msg_len)
{
	auto self(shared_from_this());
	
	m_readBuffer.resize(HEADER_SIZE + msg_len);
	boost::asio::mutable_buffers_1 buf = boost::asio::buffer(&m_readBuffer[HEADER_SIZE], msg_len);
	
	boost::asio::async_read(
		m_socket,
		buf,
		[this, self](boost::system::error_code ec, std::size_t bytes_transferred)
		{
			if (!ec)
			{
				handle_request();
				
				start_read_header();
			}
			else if (ec != boost::asio::error::operation_aborted)
			{
				m_connection_manager.stop(shared_from_this());
			}
		});
}

void Connection::handle_request()
{
	using namespace LabNet::Messages::Client;
	
	std::shared_ptr<ClientWrappedMessage> cwm = std::make_shared<ClientWrappedMessage>();
	if (cwm->ParseFromArray(&m_readBuffer[HEADER_SIZE], m_readBuffer.size() - HEADER_SIZE))
	{
		m_connection_manager.on_new_data(cwm);
	}
	else
	{
		m_logger->writeInfoEntry("invalid message");
	}
}

void Connection::refuse_conection()
{
	auto self(shared_from_this());
	
	std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> swm = std::make_shared<LabNet::Messages::Server::ServerWrappedMessage>();
	LabNet::Messages::Server::OnlyOneConnectionAllowed *onlyOne = new LabNet::Messages::Server::OnlyOneConnectionAllowed();
	swm->set_allocated_only_one_connection(onlyOne);
	
	std::vector<char> msgBuffer;
	pack_msg(swm, msgBuffer);
	
	boost::asio::async_write(m_socket,
		boost::asio::buffer(msgBuffer),
		[this, self](boost::system::error_code ec, std::size_t)
		{
			if (!ec)
			{
				m_connection_manager.stop(shared_from_this());
			}
			else if (ec != boost::asio::error::operation_aborted)
			{
				m_connection_manager.stop(shared_from_this());
			}
		});
}

void Connection::send_message(std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> mes)
{
	auto self(shared_from_this());
	
	std::vector<char> msgBuffer;
	if (pack_msg(mes, msgBuffer))
	{
		boost::asio::async_write(m_socket,
			boost::asio::buffer(msgBuffer),
			[this, self](boost::system::error_code ec, std::size_t)
			{
				m_logger->writeInfoEntry("sended");
				if (!ec)
				{

				}
				else if (ec != boost::asio::error::operation_aborted)
				{
					m_connection_manager.stop(shared_from_this());
				}
			});	
	}
}

bool Connection::pack_msg(std::shared_ptr<LabNet::Messages::Server::ServerWrappedMessage> msg, std::vector<char> &data_buffer)
{
	int size = msg->ByteSize();
	data_buffer.resize(HEADER_SIZE + size);
	if (size <= MAX_MSG_SIZE)
	{
		encode_header(size, data_buffer);
		msg->SerializeToArray(&data_buffer[HEADER_SIZE], size);
		
		return true;
	}
	
	return false;
}

void Connection::encode_header(unsigned size, std::vector<char> &data_buffer)
{
	data_buffer[0] = static_cast<char>((size >> 8) & 0xFF);
	data_buffer[1] = static_cast<char>(size & 0xFF);
}

unsigned Connection::decode_header()
{
	if (m_readBuffer.size() < HEADER_SIZE)
		return 0;
	
	unsigned msg_size = 0;
	for (unsigned i = 0; i < HEADER_SIZE; ++i)
		msg_size = msg_size * 256 + (static_cast<unsigned>(m_readBuffer[i]) & 0xFF);
	return msg_size;
}