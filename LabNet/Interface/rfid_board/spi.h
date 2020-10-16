#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>

enum SPI_IF
{
    SPI1 = 0,
    SPI2 = 1
};

void csHigh(uint8_t interface);
void csLow(uint8_t interface);
uint8_t spi_putc(uint8_t interface, uint8_t outputb);

#endif
