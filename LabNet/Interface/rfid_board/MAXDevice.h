#pragma once

#include <LoggingFacility.h>
#include <chrono>
#include <future>
#include <so_5/mbox.hpp>
#include <thread>

namespace MAX14830
{
    class MAXDevice
    {
    public:
        MAXDevice(Logger logger, so_5::mbox_t mbox);
        ~MAXDevice();

        void init(bool inverted);
        void set_phase_matrix(uint32_t antenna_phase1, uint32_t antenna_phase2, uint32_t phase_duration);
        void read_all_and_set_antenna();
        void reset_buffers();
        /// hardware reset
        void reset();
        /// turn off antenna and disable chip select
        void stop();
        //void invert(bool inverted);

    private:
        void init_uart();
        void readRXFifo(uint8_t cspin, uint8_t uart);
        void switch_phase_matrix();

        Logger _logger;
        so_5::mbox_t _parentMbox;

        uint8_t max_uart_RFIDFifo[8][4][14]; // actual readout from UART FIFO
        uint8_t max_uart_RFIDCounter[8][4]; // ring buffer

        uint32_t _antenna_phase1 = UINT32_MAX, _antenna_phase2 = UINT32_MAX;
        uint32_t _phase_duration = 250;
        char _matrix_phase = 1;
        bool _set_matrix = true;
        std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> _switch_time;
    };
}