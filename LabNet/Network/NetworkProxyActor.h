#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <LoggingFacility.h>

class ConnectionManager;

class NetworkProxyActor
{
public:
	NetworkProxyActor(Logger logger)
		: m_logger(logger)
	{
	}
	~NetworkProxyActor()
	{
	}
	
	void set_connnection_mamager(ConnectionManager* con);
	
	void connect_handler();

	void disconnect_handler();

	void data_handler(std::shared_ptr<std::vector<char>> data);
	
	void send_data(std::shared_ptr<std::vector<char>> data);

private:
	ConnectionManager* m_con;
	Logger m_logger;
};