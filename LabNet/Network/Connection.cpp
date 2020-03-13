
#include <iostream>
#include <ctime>
#include "Connection.h"
#include "ConnectionManager.h"

Connection::Connection(Logger logger, boost::asio::ip::tcp::socket socket,
	ConnectionManager &connection_manager)
	: m_logger(logger)
	, m_socket(std::move(socket))
	, m_connection_manager(connection_manager)
{
}

void Connection::start()
{
	do_read();
}

void Connection::stop()
{
	m_logger->writeInfoEntry("connection stopped");
	
	m_socket.close();
}

void Connection::do_read()
{
	auto self(shared_from_this());
	
	m_socket.async_read_some(boost::asio::buffer(m_buffer),
		[this, self](boost::system::error_code ec,
			std::size_t bytes_transferred)
		{
			if (!ec)
			{
				//std::cout << "bytes: " << bytes_transferred << " data: " << m_buffer.data() << std::endl;

				std::shared_ptr<std::vector<char>> newData = std::make_shared<std::vector<char>>(bytes_transferred);
				for (std::size_t i = 0; i < bytes_transferred; i++)
				{
					newData->at(i) = m_buffer[i];
				}
				
				m_connection_manager.on_new_data(newData);
				
				do_read();
			}
			else if (ec != boost::asio::error::operation_aborted)
			{
				m_connection_manager.stop(shared_from_this());
			}
		});
}

void Connection::refuse_conection(std::string message)
{
	auto self(shared_from_this());
	
	boost::asio::async_write(m_socket,
		boost::asio::buffer(message),
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

void Connection::send_message(std::shared_ptr<std::vector<char>> mes)
{
	auto self(shared_from_this());
	
	boost::asio::async_write(m_socket,
		boost::asio::buffer(*mes),
		[this, self](boost::system::error_code ec, std::size_t)
		{
			if (!ec)
			{

			}
			else if (ec != boost::asio::error::operation_aborted)
			{
				m_connection_manager.stop(shared_from_this());
			}
		});
}

void Connection::do_write(std::vector<char> buffer)
{
	auto self(shared_from_this());
	
	boost::asio::async_write(m_socket,
		boost::asio::buffer(buffer),
		[this, self](boost::system::error_code ec, std::size_t)
		{
			if (!ec)
			{

			}
			else if (ec != boost::asio::error::operation_aborted)
			{
				m_connection_manager.stop(shared_from_this());
			}
		});
}