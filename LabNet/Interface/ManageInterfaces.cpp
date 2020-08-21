#include "ManageInterfaces.h"
#include "InterfaceMessages.h"
#include "InitMessages.h"


Interface::ManageInterfaces::ManageInterfaces(context_t ctx)
	: so_5::agent_t(ctx)
	, _self_mbox(ctx.env().create_mbox("ManageInterfaces"))
{
	
}
		
void Interface::ManageInterfaces::so_define_agent()
{
	using namespace InitMessages;
	
	so_subscribe(_self_mbox)
		.event([this](mhood_t<init_gpio_request> msg)
	{
		if (_rfid_board_init || _gpio_wiring)
		{
			so_5::send <can_init_no>(msg->mbox);
		}
		else
		{
			_io_board_init = true;
			so_5::send <can_init_yes>(msg->mbox);
		}
	})
	.event([this](mhood_t<init_rfid_request> msg)
	{
		if (_io_board_init || _gpio_wiring)
		{
			so_5::send <can_init_no>(msg->mbox);
		}
		else
		{
			_rfid_board_init = true;
			so_5::send <can_init_yes>(msg->mbox);
		}
	})
	.event([this](mhood_t<init_gpio_wiring_request> msg)
	{
		if (_io_board_init || _rfid_board_init)
		{
			so_5::send <can_init_no>(msg->mbox);
		}
		else
		{
			_gpio_wiring = true;
			so_5::send <can_init_yes>(msg->mbox);
		}
	})
	.event([this](mhood_t<reset_interface> msg)
	{
		_rfid_board_init = false;
		_io_board_init = false;
		_gpio_wiring = false;
	});
}