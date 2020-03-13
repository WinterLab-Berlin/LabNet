#pragma once

#include <so_5/all.hpp>
#include <thread>
#include <future>
#include <vector>


class SerialPort final
{
public:
	SerialPort(const so_5::mbox_t parent, const so_5::mchain_t sendToPortBox, const int portHandler);
	~SerialPort();
	
	void send_data(std::shared_ptr<std::vector<char>> data);
	
private:
	void data_send_thread(so_5::mchain_t ch);
	
	bool stopRequested();
	
	void data_read_thread();
	
	const int m_portHandler;
	const so_5::mchain_t m_sendToPortBox;
	const so_5::mbox_t m_parent;
	std::thread m_sendWorker;
	std::thread m_readWorker;
	std::promise<void> m_exitSignal;
	std::future<void> m_futureObj;
};