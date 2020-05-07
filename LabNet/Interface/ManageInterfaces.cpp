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
		.event([this](mhood_t<init_gpio_request> msg) {
			if (_sam32_init)
			{
				so_5::send <can_init_no>(msg->mbox);
			}
			else
			{
				_gpio_init = true;
				so_5::send <can_init_yes>(msg->mbox);
			}
		})
		.event([this](mhood_t<init_sam32_request> msg) {
			if (_gpio_init)
			{
				so_5::send <can_init_no>(msg->mbox);
			}
			else
			{
				_sam32_init = true;
				so_5::send <can_init_yes>(msg->mbox);
			}
		})
		.event([this](mhood_t<reset_interface> msg) {
			_sam32_init = false;
			_gpio_init = false;
		}
	);
}