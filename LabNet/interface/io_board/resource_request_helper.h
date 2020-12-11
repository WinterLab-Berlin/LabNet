#pragma once

#include <logging_facility.h>
#include <map>
#include <so_5/all.hpp>
#include <vector>
#include "../resources/resource.h"
#include "digital_input.h"
#include "digital_output.h"

namespace LabNet::interface::io_board
{
    struct AcquireInputResult
    {
        DigitalInput dig_in;
        bool result;
    };

    struct AcquireOutputResult
    {
        DigitalOutput dig_out;
        bool result;
    };

    class ResourceRequestHelper final : public so_5::agent_t
    {
    public:
        ResourceRequestHelper(context_t ctx, so_5::mbox_t parent, log::Logger logger);
        ~ResourceRequestHelper();

    private:
        void so_define_agent() override;
        void so_evt_start() override;
        void so_evt_finish() override;

        log::Logger logger_;
        const so_5::mbox_t parent_;
        const so_5::mbox_t res_box_;
        std::map<uint16_t, std::shared_ptr<DigitalInput>> inputs_;
        std::map<uint16_t, std::shared_ptr<DigitalOutput>> outputs_;
        std::vector<resources::Resource> acquired_;
        uint16_t request_id_ = 0;
    };
}