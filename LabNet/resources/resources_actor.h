#pragma once

#include <logging_facility.h>
#include <map>
#include <so_5/all.hpp>
#include <vector>
#include "resource.h"

namespace LabNet
{
    namespace resources
    {
        struct ReserveResourcesRequest
        {
            const so_5::mbox_t reply_to;
            const so_5::mbox_t reserve_for;
            std::vector<Resource> resources;
            uint16_t request_id;
        };

        struct ReserveResourcesReply
        {
            bool result;
            uint16_t request_id;
        };

        struct ReleaseResourcesRequest
        {
            const so_5::mbox_t reply_to;
            const so_5::mbox_t reserved_for;
            std::vector<Resource> resources;
            uint16_t request_id;
        };

        struct ReleaseResourcesReply
        {
            std::vector<Resource> resources;
            uint16_t request_id;
        };

        class ResourcesActor final : public so_5::agent_t
        {
        public:
            ResourcesActor(context_t ctx, log::Logger logger);
            ~ResourcesActor();

        private:
            void so_define_agent() override;

            log::Logger logger_;
            const so_5::mbox_t self_mbox_;
            std::map<Resource, uint64_t> resources_;
        };
    }
}