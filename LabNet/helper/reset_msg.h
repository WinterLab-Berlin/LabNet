#pragma once

#include <so_5/all.hpp>

namespace LabNet::helper
{
    struct StartReset
    {
    };

    struct ResetDone
    {
    };

    struct ResetRequest
    {
        so_5::mbox_t response_box;
    };

    enum class ResponseId
    {
        DigitalOutHelper,
        ManageInterfaces
    };

    struct ResetDoneResponse
    {
        ResponseId id;
    };
}