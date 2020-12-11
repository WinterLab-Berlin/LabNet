#pragma once

#include <logging_facility.h>
#include <so_5/all.hpp>

namespace LabNet::helper
{
    class ResetHelper final : public so_5::agent_t
    {
    public:
        ResetHelper(context_t ctx, log::Logger logger);
        ~ResetHelper();

    private:
        void so_define_agent() override;

        const log::Logger logger_;
        const so_5::mbox_t server_out_box_;
        const so_5::mbox_t server_in_box_;
        const so_5::mbox_t manage_interfaces_box_;
        const so_5::mbox_t dig_out_helper_box_;

        bool reset_manage_interfaces_;
        bool reset_dig_out_helper_;
    };
}