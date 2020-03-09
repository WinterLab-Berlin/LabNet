
#include "ConnectionManager.h"
#include "../Log/easylogging++.h"

ConnectionManager::ConnectionManager()
{
}

void ConnectionManager::start(std::shared_ptr<Connection> c)
{
	if (m_connection == nullptr)
	{
		LOG(INFO) << "accept new connection";
		
		m_connection = c;
		c->start();
		
		m_connectSig();
	}
	else {
		LOG(INFO) << "only one connection possible";
		
		m_connections.insert(c);
		c->refuse_conection("too much connections");
	}
}

void ConnectionManager::stop(std::shared_ptr<Connection> c)
{
	if (m_connection == c)
	{
		c->stop();
		m_connection = nullptr;
		
		m_disconnectSig();
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
