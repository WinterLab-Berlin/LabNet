#pragma once

#include <so_5/all.hpp>
#include <LoggingFacility.h>
#include "MAXDevice.h"
#include "DataReadWorker.h"

namespace SAM
{
	class stop_worker
	{
	};
	
	class start_worker
	{
	};
	
	class SamMainActor final : public so_5::agent_t
	{
	public:
		SamMainActor(context_t ctx, const so_5::mbox_t mbox, Logger logger);
		~SamMainActor();
		
		void so_define_agent() override;
		void so_evt_start() override;
		
	private:
		void stop_worker_event(const stop_worker& ev);
		void start_worker_event(const start_worker& ev);
		
		MAX14830::MAXDevice _device;
		std::unique_ptr<MAX14830::DataReadWorker> _worker;
		Logger _logger;
		const so_5::mbox_t _parentMbox;
	};
}