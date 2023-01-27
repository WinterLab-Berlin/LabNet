#pragma once

#include <logging_facility.h>
#include <chrono>
#include <future>
#include <so_5/mbox.hpp>
#include <thread>
#include <vector>
#include <cstdint>

namespace LabNet::interface::rfid_board
{
    class MAXDevice
    {
    public:
        MAXDevice(log::Logger logger, so_5::mbox_t mbox);
        ~MAXDevice();

        void Init(bool inverted);
        void SetPhaseMatrix(uint32_t antenna_phase1, uint32_t antenna_phase2, uint32_t phase_duration);
        void ReadAllAndSetAntenna();
        void ResetBuffers();
        /// hardware reset
        void Reset();
        /// turn off antenna and disable chip select
        void Stop();
        //void invert(bool inverted);

    private:
        void InitUart();
        void ReadRXFifo(uint8_t cspin, uint8_t uart);
        void SwitchPhaseMatrix();

        log::Logger logger_;
        so_5::mbox_t parent_mbox_;

        uint8_t max_uart_RFIDFifo[8][4][16]; // actual readout from UART FIFO
        uint8_t max_uart_RFIDCounter[8][4]; // ring buffer

        uint32_t phase_duration_ = 250;
        std::vector<std::vector<uint8_t>> phases_;
        size_t matrix_phase_ = 1;
        std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> switch_time_;
    };
}
