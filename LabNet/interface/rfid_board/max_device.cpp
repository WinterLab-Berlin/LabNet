#include "max_device.h"
#include "../stream_messages.h"
#include "max_14830.h"
#include "max_14830_defs.h"
#include "spi.h"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <so_5/send_functions.hpp>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define MAXRESET 16 // pin

namespace LabNet::interface::rfid_board
{
    MAXDevice::MAXDevice(log::Logger logger, so_5::mbox_t mbox)
        : logger_(logger)
        , parent_mbox_(mbox)

    {
        using namespace std::chrono;
        typedef time_point<steady_clock, milliseconds> timePoint;
        switch_time_ = time_point_cast<timePoint::duration>(steady_clock::time_point(steady_clock::now()));

        matrix_phase_ = 0;
    }

    MAXDevice::~MAXDevice()
    {
    }

    void MAXDevice::ResetBuffers()
    {
        for (uint8_t i = 0; i < 8; i++)
        {
            for (uint8_t j = 0; j < 4; j++)
            {
                max_uart_RFIDCounter[i][j] = 0;
                for (uint8_t k = 0; k < 16; k++)
                {
                    max_uart_RFIDFifo[i][j][k] = 0;
                }
            }
        }
    }

    void MAXDevice::Reset()
    {
        logger_->WriteInfoEntry("reset max14830");

        // set reset pin to output
        pinMode(MAXRESET, OUTPUT);
        // hardware reset
        digitalWrite(MAXRESET, LOW);
        delayMicroseconds(10);
        digitalWrite(MAXRESET, HIGH);

        //TODO: there's no check here whether everything works
        // assure MAX14830 is ready
        for (uint8_t cnt1 = 0; cnt1 < 8; cnt1++)
            for (uint8_t cnt2 = 0; cnt2 < 4; cnt2++)
                max14830_isReady(cnt1, cnt2);
    }

    void MAXDevice::InitUart()
    {
        logger_->WriteInfoEntry("init uarts max14830");

        uint8_t cnt1, cnt2;

        // set baud rate on all 32 UARTs
        for (cnt1 = 0; cnt1 < 8; cnt1++)
            for (cnt2 = 0; cnt2 < 4; cnt2++)
                max14830_setBaud(cnt1, cnt2, 9600);

        // 8 bit, 1 stop bit, no parity
        for (cnt1 = 0; cnt1 < 8; cnt1++)
            for (cnt2 = 0; cnt2 < 4; cnt2++)
                max14830_setLineControl(cnt1, cnt2);

        // get baud rate and line status
        for (cnt1 = 0; cnt1 < 8; cnt1++)
        {
            //uint8_t revId = max14830_getRevisionId(cnt1);
            //printf("Revision ID (%02d): 0x%02x\n", cnt1, revId);

            for (cnt2 = 0; cnt2 < 4; cnt2++)
            {
                uint32_t baud = max14830_getBaud(cnt1, cnt2);
                //printf("Baud rate: %u\n", baud);

                uint8_t lsr = max14830_getLineStatus(cnt1, cnt2);
                //			if (lsr != 0)
                //				printf("LSR: 0x%02x UART: %u\n", lsr, cnt2);
            }
            //printf("%s", "\n");
        }

        for (cnt1 = 0; cnt1 < 8; cnt1++)
            for (cnt2 = 0; cnt2 < 4; cnt2++)
                max14830_resetFifo(cnt1, cnt2);
    }

    void MAXDevice::Init(bool inverted)
    {
        logger_->WriteInfoEntry("init max14830");

        pinMode(28, OUTPUT); // set pin 28 to OUTPUT for power supply
        digitalWrite(28, HIGH); // write HIGH to pin 28

        // set up Raspberry Pi SPI in master mode, 24MHz, mode 0
        //err = wiringPiSPISetup(SPI1, 4000000L);
        int err = wiringPiSPISetup(SPI1, 24000000L); // increased to 24MHz, default mode = 0
        if (err == -1)
        {
            logger_->WriteErrorEntry("SPI init for max14830 failed");
            throw std::runtime_error("SPI init for max14830 failed");
        }

        ResetBuffers();

        uint8_t cnt1, cnt2;

        // disable chip select
        for (cnt1 = 0; cnt1 < 8; cnt1++)
            max14830_disable(cnt1);

        // hardware reset
        Reset();

        // configure GPIO for antenna control
        for (cnt1 = 0; cnt1 < 8; cnt1++)
            for (cnt2 = 0; cnt2 < 4; cnt2++)
                max14830_configureGPIO(cnt1, cnt2);

        // turn off antennas
        for (cnt1 = 0; cnt1 < 8; cnt1++)
            for (cnt2 = 0; cnt2 < 4; cnt2++)
                max14830_setAntenna(cnt1, cnt2, false);

        // set inverted
        for (cnt1 = 0; cnt1 < 8; cnt1++)
            for (cnt2 = 0; cnt2 < 4; cnt2++)
                max14830_invertRXTXLogic(cnt1, cnt2, inverted);

        // initialize UARTs
        InitUart();

        // turn off antennas
        for (cnt1 = 0; cnt1 < 8; cnt1++)
            for (cnt2 = 0; cnt2 < 4; cnt2++)
                max14830_setAntenna(cnt1, cnt2, false);
    }

    void MAXDevice::Stop()
    {
        // switch off antenna and disable chip select
        for (uint8_t cnt1 = 0; cnt1 < 8; cnt1++)
        {
            for (uint8_t cnt2 = 0; cnt2 < 4; cnt2++)
                max14830_setAntenna(cnt1, cnt2, false);

            max14830_disable(cnt1);
        }

        matrix_phase_ = 0;
    }

    //void MAX14830::MAXDevice::invert(bool inverted)
    //{
    //    uint8_t cnt1, cnt2;
    //
    //    logger_->WriteInfoEntry("invert max14830");
    //
    //    // software reset
    //    for (cnt1 = 0; cnt1 < 8; cnt1++)
    //        for (cnt2 = 0; cnt2 < 4; cnt2++)
    //            max14830_resetUart(cnt1, cnt2);
    //
    //    // configure GPIO for antenna control
    //    for (cnt1 = 0; cnt1 < 8; cnt1++)
    //        for (cnt2 = 0; cnt2 < 4; cnt2++)
    //            max14830_configureGPIO(cnt1, cnt2);
    //
    //    // switch off antenna and set inverted
    //    for (cnt1 = 0; cnt1 < 8; cnt1++)
    //        for (cnt2 = 0; cnt2 < 4; cnt2++)
    //            max14830_setAntenna(cnt1, cnt2, false);
    //
    //    // set inverted
    //    for (cnt1 = 0; cnt1 < 8; cnt1++)
    //        for (cnt2 = 0; cnt2 < 4; cnt2++)
    //            max14830_invertRXTXLogic(cnt1, cnt2, inverted);
    //
    //    // initialize UARTs
    //    InitUart();
    //
    //    // switch on antenna
    //    for (cnt1 = 0; cnt1 < 8; cnt1++)
    //        for (cnt2 = 0; cnt2 < 4; cnt2++)
    //            max14830_setAntenna(cnt1, cnt2, true);
    //}

    void MAXDevice::ReadRXFifo(uint8_t cspin, uint8_t uart)
    {
        uint8_t RXCount, RXByte;

        // get number of words in FIFO
        RXCount = max14830_read(max_cs[cspin], max14830_RXFIFOLVL | max_uart[uart]);
        uint8_t counter = RXCount;

        while (counter > 0)
        {
            uint8_t lsr = max14830_getLineStatus(cspin, uart);
            if (lsr != 0)
            {
                //max14830_notify(cspin, uart, lsr);
            }

            RXByte = max14830_getByteFromFifo(cspin, uart);

            if (lsr & (1 << RxBreak)) // break
            {
                //fprintf(stderr, "Break detected. Ignoring byte. LSR: 0x%02x UART: %u\n", lsr, uart);
                counter--;
                continue;
            }

            // reset FIFO counter
            if (RXByte == 0x02)
                max_uart_RFIDCounter[cspin][uart] = 0;

            max_uart_RFIDFifo[cspin][uart][max_uart_RFIDCounter[cspin][uart]] = RXByte;
            max_uart_RFIDCounter[cspin][uart]++;

            uint8_t start = 0, count = 0;

            if (max_uart_RFIDCounter[cspin][uart] == 14)
            {
                if ((max_uart_RFIDFifo[cspin][uart][0] == 0x02) && (max_uart_RFIDFifo[cspin][uart][12] == 0x0a) && (max_uart_RFIDFifo[cspin][uart][13] == 0x03))
                {
                    start = 1;
                    count = 10;
                    max_uart_RFIDCounter[cspin][uart] = 0;
                }
            }
            else if (max_uart_RFIDCounter[cspin][uart] == 16)
            {
                if (max_uart_RFIDFifo[cspin][uart][0] == 0x02 && max_uart_RFIDFifo[cspin][uart][15] == 0x03)
                {
                    max_uart_RFIDCounter[cspin][uart] = 0;

                    start = 1;
                    count = 10;
                }
            }

            if (count > 0)
            {
                std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>();
                for (uint8_t l = start; l < (start + count); l++)
                {
                    data->push_back(max_uart_RFIDFifo[cspin][uart][l]);
                }
                data->push_back('\r');
                data->push_back('\n');

                so_5::send<stream_messages::NewDataFromPort>(parent_mbox_, Interfaces::RfidBoard, 4 * cspin + uart + 1, data, std::chrono::high_resolution_clock::now());
            }

            counter--;
        }
    }

    void MAXDevice::SetPhaseMatrix(uint32_t antenna_phase1, uint32_t antenna_phase2, uint32_t phase_duration)
    {
        std::vector<uint8_t> phase1;
        std::vector<uint8_t> phase2;
        for (uint8_t i = 0; i < 32; i++)
        {
            if (antenna_phase1 & (1 << i))
            {
                phase1.push_back(i);
            }
            if (antenna_phase2 & (1 << i))
            {
                phase2.push_back(i);
            }
        }
        phases_.push_back(phase1);
        phases_.push_back(phase2);

        phase_duration_ = phase_duration;
        matrix_phase_ = 0;

        using namespace std::chrono;
        typedef time_point<steady_clock, milliseconds> timePoint;
        switch_time_ = time_point_cast<timePoint::duration>(steady_clock::time_point(steady_clock::now()));
    }

    void MAXDevice::SwitchPhaseMatrix()
    {
        using namespace std::chrono;

        time_point<std::chrono::steady_clock> now = steady_clock::now();

        if (now >= switch_time_)
        {
            switch_time_ += milliseconds(phase_duration_);

            size_t next_phase = (matrix_phase_ + 1) % phases_.size();

            for (size_t i = 0; i < phases_[matrix_phase_].size(); i++)
            {
                if (std::find(phases_[next_phase].begin(), phases_[next_phase].end(), phases_[matrix_phase_][i]) == phases_[next_phase].end())
                {
                    uint8_t a = phases_[matrix_phase_][i];
                    uint8_t cnt1 = phases_[matrix_phase_][i] / 4;
                    uint8_t cnt2 = phases_[matrix_phase_][i] % 4;
                    max14830_setAntenna(cnt1, cnt2, false);
                }
            }

            matrix_phase_ = next_phase;
            for (size_t i = 0; i < phases_[matrix_phase_].size(); i++)
            {
                uint8_t a = phases_[matrix_phase_][i];
                uint8_t cnt1 = phases_[matrix_phase_][i] / 4;
                uint8_t cnt2 = phases_[matrix_phase_][i] % 4;
                max14830_setAntenna(cnt1, cnt2, true);
            }
        }
    }

    void MAXDevice::ReadAllAndSetAntenna()
    {
        for (uint8_t cnt1 = 0; cnt1 < 8; cnt1++)
            for (uint8_t cnt2 = 0; cnt2 < 4; cnt2++)
                ReadRXFifo(cnt1, cnt2);

        SwitchPhaseMatrix();
    }
};