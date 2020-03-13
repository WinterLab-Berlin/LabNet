#pragma once

#include <set>
#include <memory>
#include <boost/signals2.hpp>
#include <LoggingFacility.h>
#include "Connection.h"
#include "NetworkProxyActor.h"

class ConnectionManager
{
public:
	ConnectionManager(const ConnectionManager&) = delete;
	ConnectionManager& operator=(const ConnectionManager&) = delete;

	/// Construct a connection manager.
	ConnectionManager(Logger logger, NetworkProxyActor& proxy);

	/// Add the specified connection to the manager and start it.
	void start(std::shared_ptr<Connection> c);

	/// Stop the specified connection.
	void stop(std::shared_ptr<Connection> c);

	/// Stop all connections.
	void stop_all();
	
	void on_new_data(std::shared_ptr<std::vector<char>> data);
	
	void send_message(std::shared_ptr<std::vector<char>> mes);

private:
	Logger m_logger;
	std::shared_ptr<Connection> m_connection;
	std::set<std::shared_ptr<Connection>> m_connections;
	NetworkProxyActor m_proxy;
};

