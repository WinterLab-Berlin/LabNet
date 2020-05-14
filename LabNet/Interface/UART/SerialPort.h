#pragma once

#include <so_5/all.hpp>
#include <thread>
#include <future>
#include <vector>


class SerialPort final
{
public:
	SerialPort(const so_5::mbox_t parent, const so_5::mchain_t sendToPortBox, const int portId, const int portHandler, const int baud);
	~SerialPort();
	
	void send_data(std::shared_ptr<std::vector<char>> data);
	void activate_send_receive() { _isActive = true; };
	void deactivate_send_receive() { _isActive = false; };
	
	
private:
	void data_send_thread(so_5::mchain_t ch);
	
	bool stop_requested();
	
	void data_read_thread();
	
	
	const int _portId;
	const int _portHandler;
	const int _baud;
	const so_5::mchain_t _sendToPortBox;
	const so_5::mbox_t _parent;
	bool _isActive;
	std::thread _sendWorker;
	std::thread _readWorker;
	std::promise<void> _exitSignal;
	std::future<void> _futureObj;
};