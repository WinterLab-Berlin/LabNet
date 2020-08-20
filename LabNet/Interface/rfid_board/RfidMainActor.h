#pragma once

#include <so_5/all.hpp>
#include <LoggingFacility.h>
#include "MAXDevice.h"
#include "DataReadWorker.h"

namespace rfid_board
{
	class SamMainActor final : public so_5::agent_t
	{
	public:
		SamMainActor(context_t ctx, const so_5::mbox_t selfBox, const so_5::mbox_t parentBbox, Logger logger);
		~SamMainActor();
		
	private:
		void so_define_agent() override;
		
		so_5::state_t wait_for_init {this, "waiting for init"};
		so_5::state_t running {this, "running"};
		so_5::state_t paused {this, "paused"};
		
		std::shared_ptr<MAX14830::MAXDevice> _device;
		std::unique_ptr<MAX14830::DataReadWorker> _worker;
		Logger _logger;
		const so_5::mbox_t _selfBox;
		const so_5::mbox_t _parentMbox;
		const so_5::mbox_t _interfaces;
	};
}