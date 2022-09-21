#include "max_uart_board.h"
#include "../rfid_board/max_14830.h"
#include "../rfid_board/max_14830_defs.h"
#include "../rfid_board/spi.h"
#include "../stream_messages.h"
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <chrono>
#include <vector>

using namespace LabNet::interface::uart_board;

MaxUartBoard::MaxUartBoard(log::Logger logger, uint32_t baud, bool is_inverted)
    : logger_(logger)
{
    baud_ = baud;
    is_inverted_ = is_inverted;
}

MaxUartBoard::~MaxUartBoard()
{
}

void MaxUartBoard::Reset()
{
    logger_->WriteInfoEntry("reset max14830");

    // set reset pin to output
    pinMode(MAXRESET, OUTPUT);
    // hardware reset
    digitalWrite(MAXRESET, LOW);
    delayMicroseconds(10);
    digitalWrite(MAXRESET, HIGH);
}

void MaxUartBoard::InitUart(uint8_t cspin, uint8_t uart)
{
    max14830_setBaud(cspin, uart, baud_);
    max14830_setLineControl(cspin, uart);
    max14830_invertRXTXLogic(cspin, uart, is_inverted_);
    max14830_configureGPIO(cspin, uart);
    max14830_setGPIO(cspin, uart, 0);
}

void MaxUartBoard::Init()
{
    logger_->WriteInfoEntry("init max14830");

    pinMode(MAXPOWER, OUTPUT); // set MAXPOWER to OUTPUT for power supply
    digitalWrite(MAXPOWER, HIGH); // write HIGH to MAXPOWER

    int err = wiringPiSPISetup(SPI1, 24000000L); // increased to 24MHz, default mode = 0
    if (err == -1)
    {
        logger_->WriteErrorEntry("SPI init for max14830 failed");
        throw std::runtime_error("SPI init for max14830 failed");
    }

    uint8_t cnt1, cnt2;
    // disable chip select
    for (cnt1 = 0; cnt1 < 8; cnt1++)
        max14830_disable(cnt1);

    Reset();

    for (uint8_t cnt1 = 0; cnt1 < 8; cnt1++)
        for (uint8_t cnt2 = 0; cnt2 < 4; cnt2++)
            InitUart(cnt1, cnt2);
}

void MaxUartBoard::Stop()
{
    // switch off antenna and disable chip select
    for (uint8_t cnt1 = 0; cnt1 < 8; cnt1++)
    {
        for (uint8_t cnt2 = 0; cnt2 < 4; cnt2++)
            max14830_setGPIO(cnt1, cnt2, 0);

        max14830_disable(cnt1);
    }
}

void MaxUartBoard::ReadAllUarts(so_5::mbox_t send_to)
{
    for (uint8_t cnt1 = 0; cnt1 < 8; cnt1++)
        for (uint8_t cnt2 = 0; cnt2 < 4; cnt2++)
            ReadRxFifo(cnt1, cnt2, send_to);
}

void MaxUartBoard::ReadRxFifo(uint8_t cspin, uint8_t uart, so_5::mbox_t send_to)
{
    uint8_t rxCount, rxByte;

    // get number of words in FIFO
    rxCount = max14830_read(max_cs[cspin], max14830_RXFIFOLVL | max_uart[uart]);
    uint8_t counter = rxCount;
    std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>();

    while (counter > 0)
    {
        uint8_t lsr = max14830_getLineStatus(cspin, uart);
        if (lsr != 0)
        {
            //max14830_notify(cspin, uart, lsr);
        }

        rxByte = max14830_getByteFromFifo(cspin, uart);

        if (lsr & (1 << RxBreak)) // Break detected. Ignoring byte
        {
            counter--;
            continue;
        }

        data->push_back(rxByte);
        counter--;
    }

    if(data->size() > 0 && send_to.get() != nullptr)
    {
        so_5::send<stream_messages::NewDataFromPort>(send_to, Interfaces::UartBoard, 4 * cspin + uart + 1, data, std::chrono::high_resolution_clock::now());
    }
}

void MaxUartBoard::SetGpio(uint32_t port, uint32_t pin, bool state)
{
    if(port < 32 && pin < 4)
    {
        if(state)
            gpio_state_[port] |= (1 << pin);
        else
            gpio_state_[port] &= ~(1 << pin);

        max14830_setGPIO(port / 4, port % 4, gpio_state_[port]);
        //logger_->WriteInfoEntry(log::StringFormat("switch the pin %d %d %d", port, pin, state));
    }
}

void MaxUartBoard::WriteData(uint32_t port, std::string data)
{
    if(port > 0 && port < 33)
    {
        for (size_t i = 0; i < data.size(); i++)
        {
            max14830_sendByteToFifo((port - 1) / 4, (port - 1) % 4, data[i]);
        }
    }
}