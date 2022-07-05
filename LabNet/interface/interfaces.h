#pragma once

namespace LabNet::interface
{
    enum class Interfaces
    {
        None = 0,
        IoBoard = 1,
        RfidBoard = 2,
        GpioWiring = 3,
        Sound = 4,
        ChiBio = 5,
        BleUart = 6,
        Uart0 = 100,
        Uart1 = 101,
        Uart2 = 102,
        Uart3 = 103,
        Uart4 = 104
    };
}