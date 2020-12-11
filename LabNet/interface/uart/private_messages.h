#pragma once

namespace LabNet::interface::uart::private_messages
{
    struct TryToReconnect
    {
        const uint32_t port_id;
        const uint32_t baud;
    };

    struct SendDataComplete
    {
        const uint8_t pin;
    };

    struct PortUnexpectedClosed
    {
        const uint32_t port_id;
        const uint32_t baud;
    };

    struct PortReconnected
    {
        const uint32_t port_id;
    };

    struct SetDigitalOut
    {
        so_5::mbox_t report_box;
        uint8_t pin;
        bool state;
    };
}