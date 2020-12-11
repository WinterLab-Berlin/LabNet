#pragma once

#include <LoggingFacility.h>
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
        ResourceRequestHelper(context_t ctx, so_5::mbox_t parent, Logger logger);
        ~ResourceRequestHelper();

    private:
        void so_define_agent() override;
        void so_evt_start() override;
        void so_evt_finish() override;

        Logger _logger;
        const so_5::mbox_t _parent;
        const so_5::mbox_t _res_box;
        std::map<uint16_t, std::shared_ptr<DigitalInput>> _inputs;
        std::map<uint16_t, std::shared_ptr<DigitalOutput>> _outputs;
        std::vector<resources::Resource> _acquired;
        uint16_t request_id = 0;
    };
}