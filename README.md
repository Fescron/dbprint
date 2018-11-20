# dbprint

**DeBugPrint** is a homebrew minimal low-level `println/printf` replacement. It can be used to to print text/values to `uart`without a lot of external libraries. The end goal is to use no exernal libraries apart from the ones specific to the microcontroller.

**DeBugPrint** is originally designed for use on the `Silicion Labs Happy Gecko EFM32 board (EFM32HG322 -- TQFP48)`.

**NOTE:** There is a lot of useful simple code-examples at https://github.com/SiliconLabs/peripheral_examples

## 1 - Installation instructions (Simplicity Studo v4)

### 1.1 - Add "dbprint.c" and "dbprint.h" to your project

Open the project settings using `File > Properties > C/C++ General > Paths and Symbols`.

In the tab **"Includes"**:
1. Click `Add... > File system...`
2. Browse to the **"dbprint-inc"** folder and press OK. 
3. Tick *"Add to all languages"* and press OK.

In the tab **"Source Location"**:
1. Click `Link Folder...`
2. Tick *"Link to folder in the file system"*
3. Click `Browse...`, select the the **"dbprint-scr"** folder and press OK.

### 1.2 - Add "em_usart.c" to your project

In any *Simplicity Studio example project* (like **blink**) all of the header files for "emlib" are included but the c-files are sometimes not. This needs to be done manually:
1. In the **Project Explorer** on the left, rightclick on the **"emlib"** folder under your project and select `New > File from Template`
2. Click on `Advanced>>`and tick *"Link to file in the file system"*.
3. Click `Browse...`, go to `SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4/platform/emlib/src`, select the the **"em_usart.c"** file and press OK.
4. Press FINISH

### 1.3 - Include "dbprint.h" in your project's "main.c" file

```C
#include "dbprint.h"
```

## 2 - Methods

### 2.1 - Definitions

**Fixed baudrate = 115200 (8 databits, 1 stopbit, no parity)**.
```C
void dbprint_INIT(USART_TypeDef* pointer, uint8_t location, bool vcom, bool interrupts);
void dbAlert();
void dbClear();
void dbprint(char *message);
void dbprintUint(uint8_t radix, uint32_t value);
void dbprintInt(uint8_t radix, int32_t value);
void dbprintln(char *message);
```

### 2.2 - Usage examples

**NOTE:** VCOM is an on-board UART to USB converter alongside the *Segger J-Link debugger*, connected with microcontroller pins `PA0` (RX) `PF2` (TX).

**WARNING:** If the *Energy profiler* inside Simplicity Studio is used, printing to VCOM doesn't really work, use an external UART to USB converter while profiling the energy usage!

```C
dbprint_INIT(USART1, 4, true, false); /* Initialize UART1 on VCOM, no interrupts*/
```
```C
dbAlert(); /* Let the console make an alert sound */
dbClear(); /* Clear the console window */
```
```C
dbprint("Hello World");
dbprintln("Hello World");
```
```C
uint32_t uintValue = 42;
dbprintUint(10, uintValue); /* Decimal notation (base-10) */
dbprintUint(16, uintValue); /* Hexadecimal notation (base-16) */
```
```C
int32_t intvalue = 42;
dbprintInt(10, intvalue); /* Decimal notation (base-10) */
dbprintInt(16, intvalue); /* Hexadecimal notation (base-16) */
```


## 3 - Alternate locations of pins

In C, pin selection happens at the end of initialization methods using statements like:
```C
USART1->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC0;
```

If you use **dbprint** you don't really need to worry about this but you need to make sure you still select the correct location while using the `dbprint_INIT` method. The following location numbers and corresponding `RX`and `TX`pins for `USART0`and `USART1` are given below.

| Location |  #0  |  #1  |  #2  |  #3  |  #4  |  #5  |  #6  |
| -------- |:----:|:----:|:----:|:----:|:----:|:----:|:----:| 
| US0_RX   | PE11 |      | PC10 | PE12 | PB8  | PC1  | PC1  |
| US0_TX   | PE10 |      |      | PE13 | PB7  | PC0  | PC0  |
| US1_RX   | PC1  |      | PD6  | PD6  | PA0  | PC2  |      |
| US1_TX   | PC0  |      | PD7  | PD7  | PF2  | PC1  |      |


## 4 - Code-example that can be used in the "while(1)" loop in "main.c"
```C
/* Notified by the RX handler */
if (dbprint_rxdata)
{
      uint32_t i;
      
      /* RX Data Valid Interrupt Enable
       *   Set when data is available in the receive buffer. Cleared when the receive buffer is empty.
       *
       * TX Complete Interrupt Enable
       *   Set when a transmission has completed and no more data is available in the transmit buffer.
       *   Cleared when a new transmission starts.
       */

      /* Disable "RX Data Valid Interrupt Enable" and "TX Complete Interrupt Enable" interrupts */
      USART_IntDisable(dbpointer, USART_IEN_RXDATAV);
      USART_IntDisable(dbpointer, USART_IEN_TXC);

      /* Copy data from the RX buffer to the TX buffer */
      for (i = 0; dbprint_rx_buffer[i] != 0 && i < DBPRINT_BUFFER_SIZE-3; i++)
      {
            dbprint_tx_buffer[i] = dbprint_rx_buffer[i];
      }

      /* Add "new line" characters */
      dbprint_tx_buffer[i++] = '\r';
      dbprint_tx_buffer[i++] = '\n';
      dbprint_tx_buffer[i] = '\0';
      
      /* Reset "notification" variable */
      dbprint_rxdata = false;

      /* Enable "RX Data Valid Interrupt" and "TX Complete Interrupt" interrupts */
      USART_IntEnable(dbpointer, USART_IEN_RXDATAV);
      USART_IntEnable(dbpointer, USART_IEN_TXC);

      /* Set TX Complete Interrupt Flag (transmission has completed and no more data 
       * is available in the transmit buffer) */
      USART_IntSet(dbpointer, USART_IFS_TXC);
}
```


