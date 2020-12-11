#include "resource_request_helper.h"
#include "../resources/resources_actor.h"
#include "../resources/resource.h"

using namespace LabNet::interface::io_board;

ResourceRequestHelper::ResourceRequestHelper(context_t ctx, so_5::mbox_t parent, Logger logger)
    : so_5::agent_t(ctx)
    , _parent(parent)
    , _logger(logger)
    , _res_box(ctx.env().create_mbox("res_man"))
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
    if (_acquired.size() > 0)
    {
        so_5::send<LabNet::resources::ReleaseResourcesRequest>(_res_box, so_direct_mbox(), _parent, _acquired, static_cast<uint16_t>(0));
    }
}

void ResourceRequestHelper::so_define_agent()
{
    so_subscribe_self()
        .event([this](const so_5::mhood_t<DigitalInput> msg) {
            resources::Resource res = resources::WiringToResource(msg->pin_h);
            if (res != resources::Resource::None)
            {
                request_id++;
                std::vector<resources::Resource> res_vec;
                res_vec.push_back(res);
                so_5::send<resources::ReserveResourcesRequest>(_res_box, so_direct_mbox(), _parent, res_vec, request_id);

                _inputs[request_id] = std::make_shared<DigitalInput>(*msg);
            }
            else
            {
                so_5::send<AcquireInputResult>(_parent, DigitalInput(*msg), false);
            }
        })
        .event([this](const so_5::mhood_t<DigitalOutput> msg) {
            resources::Resource res = resources::WiringToResource(msg->pin_h);
            if (res != resources::Resource::None)
            {
                request_id++;
                std::vector<resources::Resource> res_vec;
                res_vec.push_back(res);
                so_5::send<resources::ReserveResourcesRequest>(_res_box, so_direct_mbox(), _parent, res_vec, request_id);

                _outputs[request_id] = std::make_shared<DigitalOutput>(*msg);
            }
            else
            {
                so_5::send<AcquireOutputResult>(_parent, DigitalOutput(*msg), false);
            }
        })
        .event([this](const so_5::mhood_t<resources::ReserveResourcesReply> msg) {
            auto inp_it = _inputs.find(msg->request_id);
            if (inp_it != _inputs.end())
            {
                if (msg->result)
                {
                    resources::Resource res = resources::WiringToResource(inp_it->second->pin_h);
                    _acquired.push_back(res);
                }

                _inputs.erase(inp_it);
                so_5::send<AcquireInputResult>(_parent, DigitalInput(*inp_it->second), msg->result);
            }
            else
            {
                auto out_it = _outputs.find(msg->request_id);

                if (out_it != _outputs.end())
                {
                    if (msg->result)
                    {
                        resources::Resource res = resources::WiringToResource(out_it->second->pin_h);
                        _acquired.push_back(res);
                    }

                    _outputs.erase(out_it);
                    so_5::send<AcquireOutputResult>(_parent, DigitalOutput(*out_it->second), msg->result);
                }
            }
        });
}