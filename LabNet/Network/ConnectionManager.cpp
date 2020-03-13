
#include "ConnectionManager.h"

ConnectionManager::ConnectionManager(Logger logger, NetworkProxyActor& proxy)
	: m_logger(logger)
	, m_proxy(proxy)
{
	m_proxy.set_connnection_mamager(this);
}

void ConnectionManager::start(std::shared_ptr<Connection> c)
{
	if (m_connection == nullptr)
	{
		m_logger->writeInfoEntry("accept new connection");
		
		m_connection = c;
		c->start();
		
		m_proxy.connect_handler();
	}
	else {
		m_logger->writeInfoEntry("only one connection possible");
		
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
		
		m_proxy.disconnect_handler();
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

void ConnectionManager::on_new_data(std::shared_ptr<std::vector<char>> data)
{
	m_proxy.data_handler(data);
}
	
void ConnectionManager::send_message(std::shared_ptr<std::vector<char>> mes)
{
	if (m_connection)
		m_connection->send_message(mes);
}
