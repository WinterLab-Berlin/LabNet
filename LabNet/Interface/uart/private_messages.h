#pragma once

namespace LabNet::interface::uart::private_messages
{
    struct TryToReconnect
    {
        const int port_id;
        const int baud;
    };

    struct SendDataComplete
    {
        const char pin;
    };

    struct PortUnexpectedClosed
    {
        const int port_id;
        const int baud;
    };

    struct PortReconnected
    {
        const int port_id;
    };
}