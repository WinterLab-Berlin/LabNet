#include "reset_helper.h"
#include "reset_msg.h"

using namespace LabNet::helper;

ResetHelper::ResetHelper(context_t ctx, Logger logger)
    : so_5::agent_t(ctx)
    , _logger(_logger)
    , _server_out_box(ctx.environment().create_mbox("server_out"))
    , _server_in_box(ctx.environment().create_mbox("server_in"))
    , _manage_interfaces_box(ctx.environment().create_mbox("manage_interfaces"))
    , _dig_out_helper_box(ctx.environment().create_mbox("dig_out_helper"))
{
}

ResetHelper::~ResetHelper()
{
}

void ResetHelper::so_define_agent()
{
    so_subscribe(_server_out_box)
        .event([this](const so_5::mhood_t<StartReset> msg) {
            _reset_manage_interfaces = false;
            _reset_dig_out_helper = false;

            so_5::send<ResetRequest>(_manage_interfaces_box, so_direct_mbox());
            so_5::send<ResetRequest>(_dig_out_helper_box, so_direct_mbox());
        });

    so_subscribe_self()
        .event([this](const so_5::mhood_t<ResetDoneResponse> msg) {
            if (msg->id == ResponseId::DigitalOutHelper)
            {
                _reset_dig_out_helper = true;
            }
            else if (msg->id == ResponseId::ManageInterfaces)
            {
                _reset_manage_interfaces = true;
            }

            if (_reset_dig_out_helper && _reset_manage_interfaces)
            {
                so_5::send<ResetDone>(_server_in_box);
            }
        });
}