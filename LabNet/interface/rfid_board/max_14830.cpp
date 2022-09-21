#include "max_14830.h"
#include "spi.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

uint8_t max_cs[8] = { max_cs1, max_cs2, max_cs3, max_cs4, max_cs5, max_cs6, max_cs7, max_cs8 };
uint8_t max_uart[4] = { max14830_uart0, max14830_uart1, max14830_uart2, max14830_uart3 };

void max14830_disable(uint8_t cspin)
{
    pinMode(max_cs[cspin], OUTPUT);
    digitalWrite(max_cs[cspin], HIGH); // chip select disable
}

uint8_t max14830_read(uint8_t pin, uint8_t reg)
{
    // BIT 7 BIT 6 BIT 5 BIT 4 BIT 3 BIT 2 BIT 1 BIT 0
    // W/R   U1    U0    A4    A3    A2    A1    A0

    uint8_t poutputb[2];
    int err;

    //pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW); // chip select enable

    poutputb[0] = reg & 0x7f;
    poutputb[1] = 0x00;
    err = wiringPiSPIDataRW(SPI1, poutputb, 2);
    if (err == -1)
        perror("Read/Write operation on SPI bus failed");

    digitalWrite(pin, HIGH); // chip select disable

    return poutputb[1];
}

void max14830_write(uint8_t pin, uint8_t reg, uint8_t value)
{
    // BIT 7 BIT 6 BIT 5 BIT 4 BIT 3 BIT 2 BIT 1 BIT 0
    // W/R   U1    U0    A4    A3    A2    A1    A0

    uint8_t poutputb[2];
    int err;

    //pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW); // chip select enable

    poutputb[0] = reg | 0x80;
    poutputb[1] = value;
    err = wiringPiSPIDataRW(SPI1, poutputb, 2);
    if (err == -1)
        perror("Read/Write operation on SPI bus failed");

    digitalWrite(pin, HIGH); // chip select disable
}

void max14830_isReady(uint8_t cspin, uint8_t uart)
{
    uint8_t DIVLSB;
    struct timeval tNow, tLong, tEnd;

    gettimeofday(&tNow, NULL);
    tLong.tv_sec = 500 / 1000000;
    tLong.tv_usec = 500 % 1000000;
    timeradd(&tNow, &tLong, &tEnd);

    while (timercmp(&tNow, &tEnd, <))
    {
        DIVLSB = max14830_read(max_cs[cspin], max14830_DIVLSB | max_uart[uart]);
        //printf("DIVLSB: %u at chip %u UART %u\n", DIVLSB, cspin, uart);
        if (DIVLSB == 0x01)
            return;
        gettimeofday(&tNow, NULL);
    }
}

void max14830_setBaud(uint8_t cspin, uint8_t uart, uint32_t baud)
{
    float D, fREF;

    uint16_t DIV;
    uint8_t DIVMSB, DIVLSB, FRACT;

    fREF = 3686400.0f;
    D = fREF / (16.0f * (float)baud);
    DIV = (uint16_t)truncf(D);

    FRACT = (uint8_t)roundf(16.0f * (D - DIV));
    DIVMSB = (uint8_t)((DIV & 0xff00) >> 8);
    DIVLSB = (uint8_t)(DIV & 0x00ff);

    /*
    REGISTER  ADDR  BIT7  BIT6      BIT5   BIT4   BIT3   BIT2   BIT1   BIT0
    BRGConfig 0x1B  CLKDisabl 4xMode 2xMode FRACT3 FRACT2 FRACT1 FRACT0
    DIVLSB    0x1C  Div7  Div6      Div5   Div4   Div3   Div2   Div1   Div0
    DIVMSB    0x1D  Div15 Div14     Div13  Div12  Div11  Div10  Div9   Div8
    */

    max14830_write(max_cs[cspin], max14830_BRGCONFIG | max_uart[uart], FRACT);
    max14830_write(max_cs[cspin], max14830_DIVLSB | max_uart[uart], DIVLSB);
    max14830_write(max_cs[cspin], max14830_DIVMSB | max_uart[uart], DIVMSB);
}

uint32_t max14830_getBaud(uint8_t cspin, uint8_t uart)
{
    uint8_t FRACT = max14830_read(max_cs[cspin], max14830_BRGCONFIG | max_uart[uart]);
    uint8_t DIVLSB = max14830_read(max_cs[cspin], max14830_DIVLSB | max_uart[uart]);
    uint8_t DIVMSB = max14830_read(max_cs[cspin], max14830_DIVMSB | max_uart[uart]);

    float D;
    D = (float)(DIVMSB << 8) + (float)DIVLSB + ((float)FRACT / 16.0f);

    return (uint32_t)(3686400.0f / (16.0f * D));
}

uint8_t max14830_getByteFromFifo(uint8_t cspin, uint8_t uart)
{
    return max14830_read(max_cs[cspin], max14830_RHR | max_uart[uart]);
}

void max14830_sendByteToFifo(uint8_t cspin, uint8_t uart, uint8_t byte)
{
    max14830_write(max_cs[cspin], max14830_THR | max_uart[uart], byte);
}

uint8_t max14830_getRevisionId(uint8_t cspin)
{
    max14830_setExtendedRegisterMap(cspin, true); // enable extended register map
    uint8_t revId = max14830_read(max_cs[cspin], 0x05); // 1010 0100
    max14830_setExtendedRegisterMap(cspin, false); // disable extended register map

    return revId;
}

void max14830_setLineControl(uint8_t cspin, uint8_t uart)
{
    max14830_write(max_cs[cspin], max14830_LCR | max_uart[uart], (1 << Length1) | (1 << Length0));
}

uint8_t max14830_getLineStatus(uint8_t cspin, uint8_t uart)
{
    return max14830_read(max_cs[cspin], max14830_LSR | max_uart[uart]) & 0x3f;
}

uint8_t max14830_getISRStatusReg(uint8_t cspin, uint8_t uart)
{
    return max14830_read(max_cs[cspin], max14830_ISR | max_uart[uart]);
}

void max14830_resetUart(uint8_t cspin, uint8_t uart)
{
    // software reset
    max14830_write(max_cs[cspin], max14830_MODE2 | max_uart[uart], 0x01);
    max14830_write(max_cs[cspin], max14830_MODE2 | max_uart[uart], 0x00);
}

void max14830_resetFifo(uint8_t cspin, uint8_t uart)
{
    max14830_write(max_cs[cspin], max14830_MODE2 | max_uart[uart], 0x02);
    max14830_write(max_cs[cspin], max14830_MODE2 | max_uart[uart], 0x00);
}

void max14830_setExtendedRegisterMap(uint8_t cspin, bool enable)
{
    if (enable)
        max14830_write(max_cs[cspin], max14830_GLOBLCOMND, 0xce);
    else
        max14830_write(max_cs[cspin], max14830_GLOBLCOMND, 0xcd);
}

void max14830_configureGPIO(uint8_t cspin, uint8_t uart)
{
    // GPIOConfReg GPIO0-3 PP output
    max14830_write(max_cs[cspin], max14830_GPIOCONFG | max_uart[uart], (1 << GP3Out) | (1 << GP2Out) | (1 << GP1Out) | (1 << GP0Out));
}

void max14830_setAntenna(uint8_t cspin, uint8_t uart, bool enable)
{
    if (enable)
    {
        // GPIODataReg GPIO 0-3 HIGH
        //max14830_write(max_cs[cspin], max14830_GPIODATA | max_uart[uart], (1 << GPO3Dat) | (1 << GPO2Dat) | (1 << GPO1Dat) | (1 << GPO0Dat));
        max14830_write(max_cs[cspin], max14830_GPIODATA | max_uart[uart], (1 << GPO3Dat));
    }
    else
        // GPIODataReg GPIO 0-3 LOW
        max14830_write(max_cs[cspin], max14830_GPIODATA | max_uart[uart], 0);
}

/*
 * (1 << GPO3Dat) | (1 << GPO2Dat) | (1 << GPO1Dat) | (1 << GPO0Dat)
*/
void max14830_setGPIO(uint8_t cspin, uint8_t uart, uint8_t state)
{
    max14830_write(max_cs[cspin], max14830_GPIODATA | max_uart[uart], state = (state & 0xF));
}

void max14830_invertRXTXLogic(uint8_t cspin, uint8_t uart, bool enable)
{
    uint8_t regTemp;

    regTemp = max14830_read(max_cs[cspin], max14830_IRDA | max_uart[uart]);
    if (enable)
    {
        regTemp = regTemp | (1 << 5);
        regTemp = regTemp | (1 << 4);
    }
    else
    {
        regTemp = regTemp & (uint8_t) ~(1 << 5);
        regTemp = regTemp & (uint8_t) ~(1 << 4);
    }
    max14830_write(max_cs[cspin], max14830_IRDA | max_uart[uart], regTemp);
}
