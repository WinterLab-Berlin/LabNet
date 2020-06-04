#ifndef MAX14830_H_
#define MAX14830_H_

#include <stdbool.h>
#include <stdint.h>
#include "MAX14830_defs.h"

// MAX14830 Register map defines
#define max14830_RHR          0x00
#define max14830_THR          0x00
#define max14830_IRQEN        0x01
#define max14830_ISR          0x02
#define max14830_LSRINTEN     0x03
#define max14830_LSR          0x04
#define max14830_SPCLCHRINTEN 0x05
#define max14830_SPCLCHARINT  0x06
#define max14830_STSINTEN     0x07
#define max14830_STSINT       0x08
#define max14830_MODE1        0x09
#define max14830_MODE2        0x0a
#define max14830_LCR          0x0b
#define max14830_RXTIMEOUT    0x0c
#define max14830_HDPLXDELAY   0x0d
#define max14830_IRDA         0x0e
#define max14830_FLOWLVL      0x0f
#define max14830_FIFOTRGLVL   0x10
#define max14830_TXFIFOLVL    0x11
#define max14830_RXFIFOLVL    0x12
#define max14830_FLOWCTRL     0x13
#define max14830_XON1         0x14
#define max14830_XON2         0x15
#define max14830_XOFF1        0x16
#define max14830_XOFF2        0x17
#define max14830_GPIOCONFG    0x18
#define max14830_GPIODATA     0x19
#define max14830_PLLCONFIG    0x1a
#define max14830_BRGCONFIG    0x1b
#define max14830_DIVLSB       0x1c
#define max14830_DIVMSB       0x1d
#define max14830_CLKSOURCE    0x1e

#define max14830_GLOBLCOMND   0x1f
#define max14830_TXSYNCH      0x20

// WiringPi notation
#define max_cs1 15
#define max_cs2 9
#define max_cs3 5
#define max_cs4 23
#define max_cs5 24
#define max_cs6 22
#define max_cs7 4
#define max_cs8 1

#define max14830_uart0 0x00
#define max14830_uart1 0x20
#define max14830_uart2 0x40
#define max14830_uart3 0x60

#define TXFifoMAX 128


extern uint8_t max_cs[8];
extern uint8_t max_uart[4];

void max14830_disable(uint8_t cspin);
uint8_t max14830_read(uint8_t  pin, uint8_t reg);
void max14830_write(uint8_t pin, uint8_t reg, uint8_t value);
void max14830_isReady(uint8_t cspin, uint8_t uart);
void max14830_setBaud(uint8_t cspin, uint8_t uart, uint32_t baud);
uint32_t max14830_getBaud(uint8_t cspin, uint8_t uart);
void max14830_sendByteToFifo(uint8_t cspin, uint8_t uart, uint8_t byte);
uint8_t max14830_getByteFromFifo(uint8_t cspin, uint8_t uart);
uint8_t max14830_getISRStatusReg(uint8_t cspin, uint8_t uart);
uint8_t max14830_getRevisionId(uint8_t cspin);
void max14830_setLineControl(uint8_t cspin, uint8_t uart);
uint8_t max14830_getLineStatus(uint8_t cspin, uint8_t uart);
void max14830_resetUart(uint8_t cspin, uint8_t uart);
void max14830_resetFifo(uint8_t cspin, uint8_t uart);
void max14830_configureGPIO(uint8_t cspin, uint8_t uart);
void max14830_setAntenna(uint8_t cspin, uint8_t uart, bool enable);
void max14830_setExtendedRegisterMap(uint8_t cspin, bool enable);
void max14830_invertRXTXLogic(uint8_t cspin, uint8_t uart, bool enable);

#endif
