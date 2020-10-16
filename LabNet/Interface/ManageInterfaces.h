#pragma once

#include <so_5/all.hpp>
#include <LoggingFacility.h>

namespace Interface
{
	class ManageInterfaces final : public so_5::agent_t
	{
	public:
		ManageInterfaces(context_t ctx, Logger logger, so_5::mbox_t labNetBox);
		
	private:
		void so_define_agent() override;
		void so_evt_start() override;
		
		Logger _logger;
		const so_5::mbox_t _self_mbox;
		const so_5::mbox_t _labNetBox;
		bool _io_board_init = false, _rfid_board_init = false, _gpio_wiring = false;
		so_5::mbox_t _gpioBox;
	};
}