#pragma once

#include <logging_facility.h>
#include <so_5/mbox.hpp>
#include <stdint-gcc.h>

namespace LabNet::interface::uart_board
{
    class MaxUartBoard
    {
    private:
        log::Logger logger_;
        uint8_t gpio_state_[32];
        uint32_t baud_;
        bool is_inverted_;

        /// hardware reset
        void Reset();
        // init uart
        void InitUart(uint8_t cspin, uint8_t uart);
        void ReadRxFifo(uint8_t cspin, uint8_t uart, so_5::mbox_t send_to);

    public:
        MaxUartBoard(log::Logger logger, uint32_t baud, bool is_inverted);
        ~MaxUartBoard();

        // init max14830
        void Init();
        void ReadAllUarts(so_5::mbox_t send_to);
        void Stop();
        void SetGpio(uint32_t port, uint32_t pin, bool state);
        void WriteData(uint32_t port, std::string data);
    };
}