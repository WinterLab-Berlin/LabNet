#pragma once

#include <LoggingFacility.h>

namespace MAX14830
{
	class MAXDevice
	{
	public:
		MAXDevice(Logger logger);
		~MAXDevice();
	
		void init();
	
	private:
		void reset_buffers();
		/// hardware reset
		void reset();
		/// turn off antenna and disable chip select
		void stop();
		void init_uart();
		void invert(bool inverted);
		void readRXFifo(uint8_t cspin, uint8_t uart);
		
		Logger _logger;
		
		uint8_t max_uart_RFIDBuffer[8][4][14];
		uint8_t max_uart_RFIDFifo[8][4][14];  // actual readout from UART FIFO
		uint8_t max_uart_RFIDCounter[8][4];   // ring buffer
	};
}