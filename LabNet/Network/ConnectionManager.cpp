
#include "ConnectionManager.h"
#include "server_messages.h"

ConnectionManager::ConnectionManager(Logger logger, so_5::mbox_t labNetBox)
	: m_logger(logger)
	, _labNetBox(labNetBox)
{
}

void ConnectionManager::start(std::shared_ptr<Connection> c)
{
	if (m_connection == nullptr)
	{
		m_logger->writeInfoEntry("accept new connection");
		
		m_connection = c;
		c->start();
		
		so_5::send<LabNet::network::client_connected>(_labNetBox, c);
	}
	else {
		m_logger->writeInfoEntry("only one connection possible");
		
		m_connections.insert(c);
		c->refuse_conection();
	}
}

void ConnectionManager::stop(std::shared_ptr<Connection> c)
{
	if (m_connection == c)
	{
		c->stop();
		m_connection = nullptr;
		
		so_5::send<LabNet::network::client_disconnected>(_labNetBox);
	}
	else
	{
		m_connections.erase(c);
	}
}

void ConnectionManager::stop_all()
{
	if (m_connection != nullptr)
	{
		m_connection->stop();
		m_connection = nullptr;
	}
	
	for (auto c : m_connections)
		c->stop();
	m_connections.clear();
}

