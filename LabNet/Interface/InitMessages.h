#pragma once

#include <so_5/all.hpp>

namespace Interface
{
	namespace InitMessages
	{
		struct init_gpio_request
		{
			const so_5::mbox_t mbox;
		};
		
		struct init_gpio_wiring_request
		{
			const so_5::mbox_t mbox;
		};
		
		struct init_rfid_request
		{
			const so_5::mbox_t mbox;
		};
	
		struct can_init_yes final : public so_5::signal_t {};
		struct can_init_no final : public so_5::signal_t {};
	}
}