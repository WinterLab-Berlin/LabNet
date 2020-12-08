#pragma once

namespace LabNet::interface
{
    enum class Interfaces
    {
        NONE = 0,
        IO_BOARD = 1,
        RFID_BOARD = 2,
        GPIO_WIRING = 3,
        SOUND = 4,
        UART0 = 100,
        UART1 = 101,
        UART2 = 102,
        UART3 = 103,
        UART4 = 104
    };
}