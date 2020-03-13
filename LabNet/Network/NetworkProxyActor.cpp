#include "NetworkProxyActor.h"
#include "ConnectionManager.h"

void NetworkProxyActor::set_connnection_mamager(ConnectionManager* con)
{
	m_con = con;
}
	
void NetworkProxyActor::connect_handler()
{
	std::cout << "connected" << std::endl;
}

void NetworkProxyActor::disconnect_handler()
{
	std::cout << "disconnected" << std::endl;
}

void NetworkProxyActor::data_handler(std::shared_ptr<std::vector<char>> data)
{
	std::cout << "handler: " << data->size() << " " << data->data() << std::endl;
		
	std::string str;
	for (size_t i = 0; i < data->size(); i++)
	{
		str += data->at(i);
	}
	std::cout << str << std::endl;
	
	send_data(data);
}
	
void NetworkProxyActor::send_data(std::shared_ptr<std::vector<char>> data)
{
	if (m_con)
		m_con->send_message(data);
}