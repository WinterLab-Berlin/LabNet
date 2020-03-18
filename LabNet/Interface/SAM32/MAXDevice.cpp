#include "MAXDevice.h"
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "MAX14830.h"
#include "spi.h"

#define MAXRESET 16 // pin

MAX14830::MAXDevice::MAXDevice(Logger logger)
	: _logger(logger)
{
}
MAX14830::MAXDevice::~MAXDevice()
{
}

void MAX14830::MAXDevice::reset_buffers()
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			max_uart_RFIDCounter[i][j] = 0;
			for (int k = 0; k < 14; k++)
			{
				max_uart_RFIDBuffer[i][j][k] = 0xff;
				max_uart_RFIDFifo[i][j][k] = 0;
			}
		}
	}
}

void MAX14830::MAXDevice::reset()
{
	uint8_t cnt1, cnt2;

	_logger->writeInfoEntry("reset max14830");

	// set reset pin to output
	pinMode(MAXRESET, OUTPUT);
	// hardware reset
	digitalWrite(MAXRESET, LOW);
	delayMicroseconds(10);
	digitalWrite(MAXRESET, HIGH);

	//TODO: there's no check here whether everything works
	// assure MAX14830 is ready
	for(cnt1 = 0 ; cnt1 < 8 ; cnt1++)
		for(cnt2 = 0 ; cnt2 < 4 ; cnt2++)
			max14830_isReady(cnt1, cnt2);
}

void MAX14830::MAXDevice::init_uart()
{
	_logger->writeInfoEntry("init uarts max14830");

	uint8_t cnt1, cnt2;

	// set baud rate on all 32 UARTs
	for(cnt1 = 0 ; cnt1 < 8 ; cnt1++)
		for(cnt2 = 0 ; cnt2 < 4 ; cnt2++)
			max14830_setBaud(cnt1, cnt2, 9600);

	// 8 bit, 1 stop bit, no parity
	for(cnt1 = 0 ; cnt1 < 8 ; cnt1++)
		for(cnt2 = 0 ; cnt2 < 4 ; cnt2++)
			max14830_setLineControl(cnt1, cnt2);

	// get baud rate and line status
	for(cnt1 = 0 ; cnt1 < 8 ; cnt1++)
	{
		// may be unsafe FK
		//uint8_t revId = max14830_getRevisionId(cnt1);
		//printf("Revision ID (%02d): 0x%02x\n", cnt1, revId);

		for(cnt2 = 0 ; cnt2 < 4 ; cnt2++)
		{
			uint32_t baud = max14830_getBaud(cnt1, cnt2);
			//printf("Baud rate: %u\n", baud);

			uint8_t lsr = max14830_getLineStatus(cnt1, cnt2);
			if (lsr != 0)
				printf("LSR: 0x%02x UART: %u\n", lsr, cnt2);
		}
		printf("%s", "\n");
	}

	// reset FIFO; timing with delay? FK
	for(cnt1 = 0 ; cnt1 < 8 ; cnt1++)
		for(cnt2 = 0 ; cnt2 < 4 ; cnt2++)
			max14830_resetFifo(cnt1, cnt2);
}

void MAX14830::MAXDevice::init()
{
	_logger->writeInfoEntry("init max14830");
	
	pinMode(28, OUTPUT);  // set pin 28 to OUTPUT for power supply
    digitalWrite(28, HIGH);  // write HIGH to pin 28
	
	// set up Raspberry Pi SPI in master mode, 24MHz, mode 0
    //err = wiringPiSPISetup(SPI1, 4000000L);
    int err = wiringPiSPISetup(SPI1, 24000000L);  // increased to 24MHz, default mode = 0
    if(err == -1)
	{
		_logger->writeErrorEntry("SPI init for max14830 failed");
		throw std::runtime_error("SPI init for max14830 failed");
	}
	
	reset_buffers();
	
	// disable chip select
	for(int cnt1 = 0 ; cnt1 < 8 ; cnt1++)
		max14830_disable(cnt1);
	
	// hardware reset
	reset();
	
	// configure GPIO for antenna control
	for(int cnt1 = 0 ; cnt1 < 8 ; cnt1++)
		for(int cnt2 = 0 ; cnt2 < 4 ; cnt2++)
			max14830_configureGPIO(cnt1, cnt2);
	
	// turn off antennas
	for(int cnt1 = 0 ; cnt1 < 8 ; cnt1++)
		for(int cnt2 = 0 ; cnt2 < 4 ; cnt2++)
			max14830_setAntenna(cnt1, cnt2, false);
	
	// initialize UARTs
    init_uart();
	
	// turn on antennas
	for(int cnt1 = 0 ; cnt1 < 8 ; cnt1++)
		for(int cnt2 = 0 ; cnt2 < 4 ; cnt2++)
			max14830_setAntenna(cnt1, cnt2, true);
}

void MAX14830::MAXDevice::stop()
{
	uint8_t cnt1, cnt2;

	// switch off antenna and disable chip select
	for(cnt1 = 0 ; cnt1 < 8 ; cnt1++)
	{
		for (cnt2 = 0; cnt2 < 4; cnt2++)
			max14830_setAntenna(cnt1, cnt2, false);

		max14830_disable(cnt1);
	}
}

void MAX14830::MAXDevice::invert(bool inverted)
{
	uint8_t cnt1, cnt2;

	_logger->writeInfoEntry("invert max14830");

	// software reset
	for(cnt1 = 0 ; cnt1 < 8 ; cnt1++)
		for(cnt2 = 0 ; cnt2 < 4 ; cnt2++)
			max14830_resetUart(cnt1, cnt2);

	// configure GPIO for antenna control
	for(cnt1 = 0 ; cnt1 < 8 ; cnt1++)
		for(cnt2 = 0 ; cnt2 < 4 ; cnt2++)
			max14830_configureGPIO(cnt1, cnt2);

	// switch off antenna and set inverted
	for(cnt1 = 0 ; cnt1 < 8 ; cnt1++)
		for(cnt2 = 0 ; cnt2 < 4 ; cnt2++)
			max14830_setAntenna(cnt1, cnt2, false);

	// set inverted
	for(cnt1 = 0 ; cnt1 < 8 ; cnt1++)
		for(cnt2 = 0 ; cnt2 < 4 ; cnt2++)
    		max14830_invertRXTXLogic(cnt1, cnt2, inverted);

	// initialize UARTs
	init_uart();
	
	// switch on antenna
	for(cnt1 = 0 ; cnt1 < 8 ; cnt1++)
		for(cnt2 = 0 ; cnt2 < 4 ; cnt2++)
			max14830_setAntenna(cnt1, cnt2, true);
}

void MAX14830::MAXDevice::readRXFifo(uint8_t cspin, uint8_t uart)
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
			//syslog(LOG_INFO, "LSR: %02x UART: %u\n", lsr, uart);
			//printf("LSR: 0x%02x UART: %u\n", lsr, uart);
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
		if(RXByte == 0x02)
			max_uart_RFIDCounter[cspin][uart] = 0;

		max_uart_RFIDFifo[cspin][uart][max_uart_RFIDCounter[cspin][uart]] = RXByte;
		max_uart_RFIDCounter[cspin][uart]++;

		if (max_uart_RFIDCounter[cspin][uart] == 14)
		{
			if ((max_uart_RFIDFifo[cspin][uart][0] == 0x02) &&
				(max_uart_RFIDFifo[cspin][uart][12] == 0x0a) &&
				(max_uart_RFIDFifo[cspin][uart][13] == 0x03))
			{
				max_uart_RFIDCounter[cspin][uart] = 0;

				_logger->writeInfoEntry(string_format("reader: %d rfid: %s", (4 * cspin + uart + 1), max_uart_RFIDBuffer[cspin][uart][0]));
				//memcpy(&(max_uart_RFIDBuffer[cspin][uart][0]), &(max_uart_RFIDFifo[cspin][uart][0]), 14 * sizeof(uint8_t));
				
				

//				OutputFrame.LEN = 16;     // FCT+ADR+CMD+DATA0...DATA10+CRC+END
//				OutputFrame.FCT = 0x0a;
//				OutputFrame.ADR = 107;    // should be IOFct[i].pin? FK
//				OutputFrame.CMD = 0x03;   // (uint8_t)fct_INPUT;
//				OutputFrame.Data[0] = (uint8_t)(4 * cspin + uart + 1);   // UART numbering starts with 1
//				memcpy(&(OutputFrame.Data[1]), &(max_uart_RFIDBuffer[cspin][uart][1]), 10 * sizeof(uint8_t));
//				//for (cnt = 1; cnt < 11; cnt++)
//				//	OutputFrame.Data[cnt] = max_uart_RFIDBuffer[cspin][uart][cnt];
//
//				ReturnDataTCP();   //(void*)new_sock);
			}
			else
			{
				//fprintf(stderr, "Start or stop byte missing. Reader: %u\n", (uint8_t)(4 * cspin + uart + 1));
			}
		}
		counter--;
	}
}