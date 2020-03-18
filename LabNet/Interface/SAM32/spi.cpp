#include <stdio.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "spi.h"

void csHigh(uint8_t interface)
{
    if (interface == 0)
    {
        pinMode(10, OUTPUT); // CS0
        digitalWrite(10, HIGH);
    }
    else if (interface == 1)
    {
        pinMode(11, OUTPUT); // CS1
        digitalWrite(11, HIGH);
    }
}

void csLow(uint8_t interface)
{
    if (interface == 0)
    {
        pinMode(10, OUTPUT); // CS0
        digitalWrite(10, LOW);
    }
    else if (interface == 1)
    {
        pinMode(11, OUTPUT); // CS1
        digitalWrite(11, LOW);
    }
}

uint8_t spi_putc(uint8_t interface, uint8_t outputb)
{
    uint8_t poutputb[1];
    int err;

    poutputb[0] = outputb;
    err = wiringPiSPIDataRW(interface, poutputb, 1);
    if (err)
    {
    }

    return poutputb[0];
}
