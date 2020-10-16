#ifndef MAX14830_DEFS_H_
#define MAX14830_DEFS_H_

/*
ISR Interrupt Status Register

ADDRESS: 0x02
MODE: COR (Clear on Read)
BIT 7 6 5 4 3 2 1 0
NAME CTSInt RFifoEmptyInt TFifoEmptyInt TFifoTrigInt RFifoTrigInt STSInt SpCharInt LSRErrInt
RESET 0 1 1 0 0 0 0 0
*/
#define CTSInt 7
#define RFifoEmptyInt 6
#define TFifoEmptyInt 5
#define TFifoTrigInt 4
#define RFifoTrigInt 3
#define STSInt 2
#define SpCharInt 1
#define LSRErrInt 0

/*
GPIOData GPIO Data Register

ADDRESS: 0x19
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME GPI3Dat GPI2Dat GPI1Dat GPI0Dat GPO3Dat GPO2Dat GPO1Dat GPO0Dat
RESET 0 0 0 0 0 0 0 0
*/
#define GPI3Dat 7
#define GPI2Dat 6
#define GPI1Dat 5
#define GPI0Dat 4
#define GPO3Dat 3
#define GPO2Dat 2
#define GPO1Dat 1
#define GPO0Dat 0

/*
GPIOConfg GPIO Configuration Register

ADDRESS: 0x18
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME GP3OD GP2OD GP1OD GP0OD GP3Out GP2Out GP1Out GP0Out
RESET 0 0 0 0 0 0 0 0
*/
#define GP3OD 7
#define GP2OD 6
#define GP1OD 5
#define GP0OD 4
#define GP3Out 3
#define GP2Out 2
#define GP1Out 1
#define GP0Out 0

/*
IRQEn IRQ Enable Register

ADDRESS: 0x01
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME CTSIEn RFifoEmtyIEn TFifoEmtyIEn TFifoTrgIEn RFifoTrgIEn STSIEn SpclChrIEn LSRErrIEn
RESET 0 0 0 0 0 0 0 0
*/
#define CTSIEn 7
#define RFifoEmtyIEn 6
#define TFifoEmtyIEn 5
#define TFifoTrgIEn 4
#define RFifoTrgIEn 3
#define STSIEn 2
#define SpclChrIEn 1
#define LSRErrIEn 0

/*
LSRIntEn Line Status Interrupt Enable Register

ADDRESS: 0x03
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME NoiseIntEn RBreakIEn FrameErrIEn ParityIEn ROverrIEn RTimoutIEn
RESET 0 0 0 0 0 0 0 0
*/
#define NoiseIntEn 5
#define RBreakIEn 4
#define FrameErrIEn 3
#define ParityIEn 2
#define ROverrIEn 1
#define RTimoutIEn 0

/*
LSR Line Status Register

ADDRESS: 0x04
MODE: R
BIT 7 6 5 4 3 2 1 0
NAME CTSbit RxNoise RxBreak FrameErr RxParityErr RxOverrun RTimeout
RESET X 0 0 0 0 0 0 0
*/
#define CTSbit 7
#define RxNoise 5
#define RxBreak 4
#define FrameErr 3
#define RxParityErr 2
#define RxOverrun 1
#define RTimeout 0

/*
SpclChrIntEn Special Character Interrupt Enable Register

ADDRESS: 0x05
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME MltDrpIntEn BREAKIntEn XOFF2IntEn XOFF1IntEn XON2IntEn XON1IntEn
RESET 0 0 0 0 0 0 0 0
*/
#define MltDrpIntEn 5
#define BREAKIntEn 4
#define XOFF2IntEn 3
#define XOFF1IntEn 2
#define XON2IntEn 1
#define XON1IntEn 0

/*
SpclCharInt Special Character Interrupt Register

ADDRESS: 0x06
MODE: COR
BIT 7 6 5 4 3 2 1 0
NAME MultiDropInt BREAKInt XOFF2Int XOFF1Int XON2Int XON1Int
RESET 0 0 0 0 0 0 0 0
*/
#define MultiDropInt 5
#define BREAKInt 4
#define XOFF2Int 3
#define XOFF1Int 2
#define XON2Int 1
#define XON1Int 0

/*
STSIntEn STS Interrupt Enable Register

ADDRESS: 0x07
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME ClockRdyIntEn GPI3IntEn GPI2IntEn GPI1IntEn GPI0IntEn
RESET 0 0 0 0 0 0 0 0
*/
#define ClockRdyIntEn 5
#define GPI3IntEn 3
#define GPI2IntEn 2
#define GPI1IntEn 1
#define GPI0IntEn 0

/*
STSInt Status Interrupt Register

ADDRESS: 0x08
MODE: R/COR
BIT 7 6 5 4 3 2 1 0
NAME ClockReady GPI3Int GPI2Int GPI1Int GPI0Int
RESET 0 0 0 0 0 0 0 0
*/
#define ClockReady 5
#define GPI3Int 3
#define GPI2Int 2
#define GPI1Int 1
#define GPI0Int 0

/*
MODE1 Register

ADDRESS: 0x09
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME IRQSel TrnscvCtrl RTSHiZ TxHiZ TxDisabl RxDisabl
RESET 0 0 0 0 0 0 0 0
*/
#define IRQSel 7
#define TrnscvCtrl 4
#define RTSHiZ 3
#define TxHiZ 2
#define TxDisabl 1
#define RxDisabl 0

/*
MODE2 Register

ADDRESS: 0x0a
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME EchoSuprs MultiDrop Loopback SpecialChr RxEmtyInv RxTrigInv FIFORst RST
RESET 0 0 0 0 0 0 0 0
*/
#define EchoSuprs 7
#define MultiDrop 6
#define Loopback 5
#define SpecialChr 4
#define RxEmtyInv 3
#define RxTrigInv 2
#define FIFORst 1
#define RST 0

/*
LCR Line Control Register

ADDRESS: 0x0b
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME RTSbit TxBreak ForceParity EvenParity ParityEn StopBits Length1 Length0
RESET 0 0 0 0 0 1 0 1
*/
#define RTSbit 7
#define TxBreak 6
#define ForceParity 5
#define EvenParity 4
#define ParityEn 3
#define StopBits 2
#define Length1 1
#define Length0 0

/*
RxTimeOut Receiver Timeout Register

ADDRESS: 0x0c
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME TimOut7 TimOut6 TimOut5 TimOut4 TimOut3 TimOut2 TimOut1 TimOut0
RESET 0 0 0 0 0 0 0 0
*/
#define TimOut7 7
#define TimOut6 6
#define TimOut5 5
#define TimOut4 4
#define TimOut3 3
#define TimOut2 2
#define TimOut1 1
#define TimOut0 0

/*
HDplxDelay Register

ADDRESS: 0x0d
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME Setup3 Setup2 Setup1 Setup0 Hold3 Hold2 Hold1 Hold0
RESET 0 0 0 0 0 0 0 0
*/
#define Setup3 7
#define Setup2 6
#define Setup1 5
#define Setup0 4
#define Hold3 3
#define Hold2 2
#define Hold1 1
#define Hold0 0

/*
IrDA Register

ADDRESS: 0x0e
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME TxInv RxInv MIR RTSInvert SIR IrDAEn
RESET 0 0 0 0 0 0 0 0
*/
#define TxInv 5
#define RxInv 4
#define MIR 3
#define RTSInvert 2
#define SIR 1
#define IrDAEn 0

/*
FlowLvl Flow Level Register

ADDRESS: 0x0f
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME Resume3 Resume2 Resume1 Resume0 Halt3 Halt2 Halt1 Halt0
RESET 0 0 0 0 0 0 0 0
*/
#define Resume3 7
#define Resume2 6
#define Resume1 5
#define Resume0 4
#define Halt3 3
#define Halt2 2
#define Halt1 1
#define Halt0 0

/*
FIFOTrigLvl FIFO Interrupt Trigger Level Register

ADDRESS: 0x10
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME RxTrig3 RxTrig2 RxTrig1 RxTrig0 TxTrig3 TxTrig2 TxTrig1 TxTrig0
RESET 1 1 1 1 1 1 1 1
*/
#define RxTrig3 7
#define RxTrig2 6
#define RxTrig1 5
#define RxTrig0 4
#define TxTrig3 3
#define TxTrig2 2
#define TxTrig1 1
#define TxTrig0 0

/*
TxFIFOLvl Transmit FIFO Level Register
Amount of words in the TXFifo
ADDRESS: 0x11
MODE: R
BIT 7 6 5 4 3 2 1 0
NAME TxFL7 TxFL6 TxFL5 TxFL4 TxFL3 TxFL2 TxFL1 TxFL0
RESET 0 0 0 0 0 0 0 0
*/
#define TxFL7 7
#define TxFL6 6
#define TxFL5 5
#define TxFL4 4
#define TxFL3 3
#define TxFL2 2
#define TxFL1 1
#define TxFL0 0

/*
RxFIFOLvl Receive FIFO Level Register
Amount of words in the RXFifo
ADDRESS: 0x12
MODE: R
BIT 7 6 5 4 3 2 1 0
NAME RxFL7 RxFL6 RxFL5 RxFL4 RxFL3 RxFL2 RxFL1 RxFL0
RESET 0 0 0 0 0 0 0 0
*/
#define RxFL7 7
#define RxFL6 6
#define RxFL5 5
#define RxFL4 4
#define RxFL3 3
#define RxFL2 2
#define RxFL1 1
#define RxFL0 0

/*
FlowCtrl Flow Control Register

ADDRESS: 0x13
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME SwFlow3 SwFlow2 SwFlow1 SwFlow0 SwFlowEn GPIAddr AutoCTS AutoRTS
RESET 0 0 0 0 0 0 0 0
*/
#define SwFlow3 7
#define SwFlow2 6
#define SwFlow1 5
#define SwFlow0 4
#define SwFlowEn 3
#define GPIAddr 2
#define AutoCTS 1
#define AutoRTS 0

/*
GPIOConfg GPIO Configuration Register

ADDRESS: 0x18
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME GP3OD GP2OD GP1OD GP0OD GP3Out GP2Out GP1Out GP0Out
RESET 0 0 0 0 0 0 0 0
*/
#define GP3OD 7
#define GP2OD 6
#define GP1OD 5
#define GP0OD 4
#define GP3Out 3
#define GP2Out 2
#define GP1Out 1
#define GP0Out 0

/*
GPIOData GPIO Data Register

ADDRESS: 0x19
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME GPI3Dat GPI2Dat GPI1Dat GPI0Dat GPO3Dat GPO2Dat GPO1Dat GPO0Dat
RESET 0 0 0 0 0 0 0 0
*/
#define GPI3Dat 7
#define GPI2Dat 6
#define GPI1Dat 5
#define GPI0Dat 4
#define GPO3Dat 3
#define GPO2Dat 2
#define GPO1Dat 1
#define GPO0Dat 0

/*
UART  GPI3Dat/GPO3Dat GPI2Dat/GPO2Dat GPI1Dat/GPO1Dat GPI0Dat/GPO0Dat
UART0 GPIO3           GPIO2           GPIO1           GPIO0
UART1 GPIO7           GPIO6           GPIO5           GPIO4
UART2 GPIO11          GPIO10          GPIO9           GPIO8
UART3 GPIO15          GPIO14          GPIO13          GPIO12
*/

/*
PLLConfig PLL Configuration Register

ADDRESS: 0x1a
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME PLLFactor1 PLLFactor0 PreDiv5 PreDiv4 PreDiv3 PreDiv2 PreDiv1 PreDiv0
RESET 0 0 0 0 0 0 0 1
*/

/*
BRGConfig Baud-Rate Generator Configuration Register

ADDRESS: 0x1b
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME CLKDisabl 4xMode 2xMode FRACT3 FRACT2 FRACT1 FRACT0
RESET 0 0 0 0 0 0 0 0
*/

/*
DIVLSB Baud-Rate Generator LSB Divisor Register

ADDRESS: 0x1c
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME Div7 Div6 Div5 Div4 Div3 Div2 Div1 Div0
RESET 0 0 0 0 0 0 0 1
*/

/*
DIVMSB Baud-Rate Generator MSB Divisor Register

ADDRESS: 0x1d
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME Div15 Div14 Div13 Div12 Div11 Div10 Div9 Div8
RESET 0 0 0 0 0 0 0 0
*/

/*
CLKSource Clock Source Register

ADDRESS: 0x1e
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME CLKtoRTS PLLBypass PLLEn CrystalEn
RESET 0 0 0 0 1 0 0 0
*/

/*
GlobalIRQ Global IRQ Register

ADDRESS: 0x1f
MODE: R
BIT 7 6 5 4 3 2 1 0
NAME IRQ3 IRQ2 IRQ1 IRQ0
RESET 0 0 0 0 1 1 1 1
*/

/*
GloblComnd Global Command Register

ADDRESS: 0x1f
MODE: W
BIT 7 6 5 4 3 2 1 0
NAME GlbCom7 GlbCom6 GlbCom5 GlbCom4 GlbCom3 GlbCom2 GlbCom1 GlbCom0
*/

/*
GloblComnd[7:0] COMMAND DESCRIPTION
0xE0 Tx Command 0
0xE1 Tx Command 1
0xE2 Tx Command 2
0xE3 Tx Command 3
0xE4 Tx Command 4
0xE5 Tx Command 5
0xE6 Tx Command 6
0xE7 Tx Command 7
0xE8 Tx Command 8
0xE9 Tx Command 9
0xEA Tx Command 10
0xEB Tx Command 11
0xEC Tx Command 12
0xED Tx Command 13
0xEE Tx Command 14
0xEF Tx Command 15
0xCE Enable extended register map access
0xCD Disable extended register map access
*/

/*
TxSynch Transmitter Synchronization Register

ADDRESS: 0x20
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME CLKtoGPIO TxAutoDis TrigDelay SynchEn TrigSel3 TrigSel2 TrigSel1 TrigSel0
RESET 0 0 0 0 0 0 0 0
*/

/*
SynchDelay1 Synchronization Delay Register 1
TIMER2 Timer Register 2
ADDRESS: 0x21
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME SDelay7 SDelay6 SDelay5 SDelay4 SDelay3 SDelay2 SDelay1 SDelay0
RESET 0 0 0 0 0 0 0 0
*/
#define SDelay7 7
#define SDelay6 6
#define SDelay5 5
#define SDelay4 4
#define SDelay3 3
#define SDelay2 2
#define SDelay1 1
#define SDelay0 0

/*
SynchDelay2 Synchronization Delay Register 2

ADDRESS: 0x22
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME SDelay15 SDelay14 SDelay13 SDelay12 SDelay11 SDelay10 SDelay9 SDelay8
RESET 0 0 0 0 0 0 0 0
*/
#define SDelay15 7
#define SDelay14 6
#define SDelay13 5
#define SDelay12 4
#define SDelay11 3
#define SDelay10 2
#define SDelay9 1
#define SDelay8 0

/*
TIMER1 Timer Register 1

ADDRESS: 0x23
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME Timer7 Timer6 Timer5 Timer4 Timer3 Timer2 Timer1 Timer0
RESET 0 0 0 0 0 0 0 0
*/
#define Timer7 7
#define Timer6 6
#define Timer5 5
#define Timer4 4
#define Timer3 3
#define Timer2 2
#define Timer1 1
#define Timer0 0

/*
TIMER2 Timer Register 2

ADDRESS: 0x24
MODE: R/W
BIT 7 6 5 4 3 2 1 0
NAME TmrToGPIO Timer14 Timer13 Timer12 Timer11 Timer10 Timer9 Timer8
RESET 0 0 0 0 0 0 0 0
*/
#define TmrToGPIO 7
#define Timer14 6
#define Timer13 5
#define Timer12 4
#define Timer11 3
#define Timer10 2
#define Timer9 1
#define Timer8 0

/*
TxSynch Transmitter Synchronization Register

MODE: R/W
RESET 0 0 0 0 0 0 0 0
*/
#define max14830_TXSYNCH_SPI 0x00
#define CLKtoGPIO 7
#define TxAutoDis 6
#define TrigDelay 5
#define SynchEn 4
#define TrigSel3 3
#define TrigSel2 2
#define TrigSel1 1
#define TrigSel0 0

#endif
