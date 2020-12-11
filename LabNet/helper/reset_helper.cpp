#include "reset_helper.h"
#include "reset_msg.h"

using namespace LabNet::helper;

ResetHelper::ResetHelper(context_t ctx, log::Logger logger)
    : so_5::agent_t(ctx)
    , logger_(logger)
    , server_out_box_(ctx.environment().create_mbox("server_out"))
    , server_in_box_(ctx.environment().create_mbox("server_in"))
    , manage_interfaces_box_(ctx.environment().create_mbox("manage_interfaces"))
    , dig_out_helper_box_(ctx.environment().create_mbox("dig_out_helper"))
{
}

ResetHelper::~ResetHelper()
{
}

void ResetHelper::so_define_agent()
{
    so_subscribe(server_out_box_)
        .event([this](const so_5::mhood_t<StartReset> msg) {
            reset_manage_interfaces_ = false;
            reset_dig_out_helper_ = false;

            so_5::send<ResetRequest>(manage_interfaces_box_, so_direct_mbox());
            so_5::send<ResetRequest>(dig_out_helper_box_, so_direct_mbox());
        });

    so_subscribe_self()
        .event([this](const so_5::mhood_t<ResetDoneResponse> msg) {
            if (msg->id == ResponseId::DigitalOutHelper)
            {
                reset_dig_out_helper_ = true;
            }
            else if (msg->id == ResponseId::ManageInterfaces)
            {
                reset_manage_interfaces_ = true;
            }

            if (reset_dig_out_helper_ && reset_manage_interfaces_)
            {
                so_5::send<ResetDone>(server_in_box_);
            }
        });
}