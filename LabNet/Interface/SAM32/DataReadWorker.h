#pragma once

#include <chrono>
#include <thread>
#include <future>
#include <so_5/all.hpp>
#include "MAXDevice.h"

namespace MAX14830
{
	class DataReadWorker
	{
	public:
		DataReadWorker(MAXDevice& dev);
		~DataReadWorker();
		
	private:
		MAXDevice& _device;
		std::thread _readWorker;
		std::promise<void> _exitSignal;
		std::future<void> _futureObj;
		
		
		bool stop_requested();
	
		void read_rfid_thread();
	};
}