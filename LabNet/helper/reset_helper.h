#pragma once

#include <LoggingFacility.h>
#include <so_5/all.hpp>

namespace LabNet::helper
{
    class ResetHelper final : public so_5::agent_t
    {
    public:
        ResetHelper(context_t ctx, Logger logger);
        ~ResetHelper();

    private:
        void so_define_agent() override;

        const Logger _logger;
        const so_5::mbox_t _server_out_box;
        const so_5::mbox_t _server_in_box;
        const so_5::mbox_t _manage_interfaces_box;
        const so_5::mbox_t _dig_out_helper_box;

        bool _reset_manage_interfaces;
        bool _reset_dig_out_helper;
    };
}