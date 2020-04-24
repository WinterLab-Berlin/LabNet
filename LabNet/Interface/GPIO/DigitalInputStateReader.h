#pragma once

#include <so_5/all.hpp>
#include <thread>
#include <future>
#include <map>
#include "DigitalInput.h"

namespace GPIO
{
	class DigitalInputStateReader
	{
		
	public:
		DigitalInputStateReader(const so_5::mbox_t parent, std::map<int, DigitalInput>& inputs);
		~DigitalInputStateReader();
		
	private:
		void state_read_thread();
	
		bool stop_requested();
	
		void data_read_thread();
	
		std::map<int, DigitalInput> _inputs;
		const so_5::mbox_t _parent;
		
		std::thread _readWorker;
		std::promise<void> _exitSignal;
		std::future<void> _futureObj;
	};
}