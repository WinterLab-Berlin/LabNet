#include "resources_actor.h"

namespace LabNet::resources
{
    resources_actor::resources_actor(context_t ctx, Logger logger)
        : so_5::agent_t(ctx)
        , _self_mbox(ctx.env().create_mbox("res_man"))
        , _logger(logger)
        , _resources()
    {
    }

    resources_actor::~resources_actor()
    {
    }

    void resources_actor::so_define_agent()
    {
        so_default_state()
            .event(_self_mbox,
                [this](const mhood_t<ReserveResourcesRequest> msg) {
                    if (msg->reply_to && msg->reserve_for)
                    {
                        uint64_t id = msg->reserve_for->id();
                        bool can_reserve = true;
                        for (auto& r : msg->resources)
                        {
                            if (r != Resource::None)
                            {
                                auto it = _resources.find(r);
                                if (it != _resources.end() && it->second != id)
                                {
                                    can_reserve = false;
                                    break;
                                }
                            }
                        }

                        if (can_reserve)
                        {
                            for (auto& r : msg->resources)
                            {
                                _resources[r] = id;
                            }
                        }

                        so_5::send<ReserveResourcesReply>(msg->reply_to, can_reserve, msg->request_id);
                    }
                })
            .event(_self_mbox,
                [this](const mhood_t<ReleaseResourcesRequest> msg) {
                    if (msg->reply_to && msg->reserved_for)
                    {
                        uint64_t id = msg->reserved_for->id();
                        std::vector<Resource> released;

                        for (auto& r : msg->resources)
                        {
                            auto it = _resources.find(r);
                            if (it == _resources.end() || it->second == id)
                            {
                                _resources.erase(r);
                                released.push_back(r);
                            }
                        }

                        so_5::send<ReleaseResourcesReply>(msg->reply_to, released, msg->request_id);
                    }
                });
    }
}