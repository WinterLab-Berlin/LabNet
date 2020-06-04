#pragma once

#include <so_5/all.hpp>
#include <LoggingFacility.h>

namespace Interface
{
	class ManageInterfaces final : public so_5::agent_t
	{
	public:
		ManageInterfaces(context_t ctx);
		
	private:
		void so_define_agent() override;
		
		Logger _logger;
		const so_5::mbox_t _self_mbox;
		bool _gpio_init = false, _rfid_init = false;
	};
}