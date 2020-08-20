#pragma once

#define BOOST_BIND_NO_PLACEHOLDERS

#include <set>
#include <memory>
#include <boost/signals2.hpp>
#include <LoggingFacility.h>
#include <so_5/all.hpp>
#include "Connection.h"
#include "ProtocolAll.h"

class ConnectionManager
{
public:
	ConnectionManager(const ConnectionManager&) = delete;
	ConnectionManager& operator=(const ConnectionManager&) = delete;

	/// Construct a connection manager.
	ConnectionManager(Logger logger, so_5::mbox_t labNetBox);

	/// Add the specified connection to the manager and start it.
	void start(std::shared_ptr<Connection> c);

	/// Stop the specified connection.
	void stop(std::shared_ptr<Connection> c);

	/// Stop all connections.
	void stop_all();
	
	void on_new_data(std::shared_ptr<LabNetProt::Client::ClientWrappedMessage> mes);

private:
	Logger m_logger;
	std::shared_ptr<Connection> m_connection;
	std::set<std::shared_ptr<Connection>> m_connections;
	so_5::mbox_t _labNetBox;
};

