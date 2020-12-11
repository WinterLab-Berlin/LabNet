#include "resource_request_helper.h"
#include "../resources/resources_actor.h"
#include "../resources/resource.h"

using namespace LabNet::interface::gpio_wiring;

ResourceRequestHelper::ResourceRequestHelper(context_t ctx, so_5::mbox_t parent, log::Logger logger)
    : so_5::agent_t(ctx)
    , parent_(parent)
    , logger_(logger)
    , res_box_(ctx.env().create_mbox("res_man"))
{
}

ResourceRequestHelper::~ResourceRequestHelper()
{
}

void ResourceRequestHelper::so_evt_start()
{
}

void ResourceRequestHelper::so_evt_finish()
{
    if (acquired_.size() > 0)
    {
        so_5::send<LabNet::resources::ReleaseResourcesRequest>(res_box_, so_direct_mbox(), parent_, acquired_, static_cast<uint16_t>(0));
    }
}

void ResourceRequestHelper::so_define_agent()
{
    so_subscribe_self()
        .event([this](const so_5::mhood_t<DigitalInput> msg) {
            resources::Resource res = resources::WiringToResource(msg->pin);
            if (res != resources::Resource::None)
            {
                request_id_++;
                std::vector<resources::Resource> res_vec;
                res_vec.push_back(res);
                so_5::send<resources::ReserveResourcesRequest>(res_box_, so_direct_mbox(), parent_, res_vec, request_id_);

                inputs_[request_id_] = std::make_shared<DigitalInput>(*msg);
            }
            else
            {
                so_5::send<AcquireInputResult>(parent_, DigitalInput(*msg), false);
            }
        })
        .event([this](const so_5::mhood_t<DigitalOutput> msg) {
            resources::Resource res = resources::WiringToResource(msg->pin);
            if (res != resources::Resource::None)
            {
                request_id_++;
                std::vector<resources::Resource> res_vec;
                res_vec.push_back(res);
                so_5::send<resources::ReserveResourcesRequest>(res_box_, so_direct_mbox(), parent_, res_vec, request_id_);

                outputs_[request_id_] = std::make_shared<DigitalOutput>(*msg);
            }
            else
            {
                so_5::send<AcquireOutputResult>(parent_, DigitalOutput(*msg), false);
            }
        })
        .event([this](const so_5::mhood_t<resources::ReserveResourcesReply> msg) {
            auto inp_it = inputs_.find(msg->request_id);
            if (inp_it != inputs_.end())
            {
                if (msg->result)
                {
                    resources::Resource res = resources::WiringToResource(inp_it->second->pin);
                    acquired_.push_back(res);
                }

                inputs_.erase(inp_it);
                so_5::send<AcquireInputResult>(parent_, DigitalInput(*inp_it->second), msg->result);
            }
            else
            {
                auto out_it = outputs_.find(msg->request_id);

                if (out_it != outputs_.end())
                {
                    if (msg->result)
                    {
                        resources::Resource res = resources::WiringToResource(out_it->second->pin);
                        acquired_.push_back(res);
                    }

                    outputs_.erase(out_it);
                    so_5::send<AcquireOutputResult>(parent_, DigitalOutput(*out_it->second), msg->result);
                }
            }
        });
}