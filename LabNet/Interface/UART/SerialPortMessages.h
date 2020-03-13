#pragma once

#include <string>
#include <vector>
#include <memory>

struct port_unexpected_closed
{
	const int handle;
};

struct new_data_from_port
{
	const int handle;
	std::shared_ptr<std::vector<char>> data;
};

struct send_data_to_port
{
	const int port_id;
	std::shared_ptr<std::vector<char>> data;
};

struct init_port
{
	const int port;
	const int baud;
	const so_5::mbox_t m_mbox;
};

struct init_port_error
{
	const int port;
};

struct init_port_success
{
	const int port;
};