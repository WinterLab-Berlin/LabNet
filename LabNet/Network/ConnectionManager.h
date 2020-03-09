#pragma once

#include <set>
#include <memory>
#include <boost/signals2.hpp>
#include "Connection.h"

class ConnectionManager
{
public:
	ConnectionManager(const ConnectionManager&) = delete;
	ConnectionManager& operator=(const ConnectionManager&) = delete;

	/// Construct a connection manager.
	ConnectionManager();

	/// Add the specified connection to the manager and start it.
	void start(std::shared_ptr<Connection> c);

	/// Stop the specified connection.
	void stop(std::shared_ptr<Connection> c);

	/// Stop all connections.
	void stop_all();
	
	void on_new_data(std::shared_ptr<std::vector<char>> data)
	{
		m_data(data);
	}
	
	boost::signals2::connection add_connect_handler(const boost::signals2::signal<void()>::slot_type &subscriber)
	{
		return m_connectSig.connect(subscriber);
	}
	boost::signals2::connection add_disconnect_handler(const boost::signals2::signal<void()>::slot_type &subscriber)
	{
		return m_disconnectSig.connect(subscriber);
	}
	boost::signals2::connection add_data_received_handler(const boost::signals2::signal<void(std::shared_ptr<std::vector<char>>)>::slot_type &subscriber)
	{
		return m_data.connect(subscriber);
	}

private:
	std::shared_ptr<Connection> m_connection;
	std::set<std::shared_ptr<Connection>> m_connections;
	
	boost::signals2::signal<void()> m_connectSig;
	boost::signals2::signal<void()> m_disconnectSig;
	boost::signals2::signal<void(std::shared_ptr<std::vector<char>>)> m_data;
};

