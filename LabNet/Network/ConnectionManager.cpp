
#include "ConnectionManager.h"
#include "../LabNetMainActorMessages.h"

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
		
		so_5::send<LabNet::Connected>(_labNetBox, c);
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
		
		so_5::send<LabNet::Disconnected>(_labNetBox);
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

void ConnectionManager::on_new_data(std::shared_ptr<LabNet::Client::ClientWrappedMessage> mes)
{
	so_5::send<std::shared_ptr<LabNet::Client::ClientWrappedMessage>>(_labNetBox, mes);
}
