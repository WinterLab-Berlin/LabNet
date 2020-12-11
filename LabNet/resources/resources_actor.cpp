#include "resources_actor.h"

namespace LabNet::resources
{
    ResourcesActor::ResourcesActor(context_t ctx, log::Logger logger)
        : so_5::agent_t(ctx)
        , self_mbox_(ctx.env().create_mbox("res_man"))
        , logger_(logger)
        , resources_()
    {
    }

    ResourcesActor ::~ResourcesActor ()
    {
    }

    void ResourcesActor ::so_define_agent()
    {
        so_default_state()
            .event(self_mbox_,
                [this](const mhood_t<ReserveResourcesRequest> msg) {
                    if (msg->reply_to && msg->reserve_for)
                    {
                        uint64_t id = msg->reserve_for->id();
                        bool can_reserve = true;
                        for (auto& r : msg->resources)
                        {
                            if (r != Resource::None)
                            {
                                auto it = resources_.find(r);
                                if (it != resources_.end() && it->second != id)
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
                                resources_[r] = id;
                            }
                        }

                        so_5::send<ReserveResourcesReply>(msg->reply_to, can_reserve, msg->request_id);
                    }
                })
            .event(self_mbox_,
                [this](const mhood_t<ReleaseResourcesRequest> msg) {
                    if (msg->reply_to && msg->reserved_for)
                    {
                        uint64_t id = msg->reserved_for->id();
                        std::vector<Resource> released;

                        for (auto& r : msg->resources)
                        {
                            auto it = resources_.find(r);
                            if (it == resources_.end() || it->second == id)
                            {
                                resources_.erase(r);
                                released.push_back(r);
                            }
                        }

                        so_5::send<ReleaseResourcesReply>(msg->reply_to, released, msg->request_id);
                    }
                });
    }
}