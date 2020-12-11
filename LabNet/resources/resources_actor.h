#pragma once

#include <LoggingFacility.h>
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

        class resources_actor final : public so_5::agent_t
        {
        public:
            resources_actor(context_t ctx, Logger logger);
            ~resources_actor();

        private:
            void so_define_agent() override;

            Logger _logger;
            const so_5::mbox_t _self_mbox;
            std::map<Resource, uint64_t> _resources;
        };
    }
}