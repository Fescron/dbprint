# dbprint

**DeBugPrint** is a homebrew minimal low-level `println/printf` replacement. It can be used to to print text/values to `uart`without a lot of external libraries. The end goal is to use no exernal libraries apart from the ones specific to the microcontroller.

**DeBugPrint** is originally designed for use on the `Silicion Labs Happy Gecko EFM32 board (EFM32HG322 -- TQFP48)`.


## Installation instructions (Simplicity Studo v4)

`File > Properties > C/C++ General > Paths and Symbols`

In the tab **"Includes"**:
1. Click `Add... > File system...`
2. Browse to the **"dbprint-inc"** folder and press OK. 
3. Tick *"Add to all languages"* and press OK.

In the tab **"Source Location"**:
1. Click `Link Folder...`
2. Tick *"Link to folder in the file system"*
3. Click `Browse...`, select the the **"dbprint-scr"** folder and press OK.


## Methods

### Definitions

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

### Usage examples

VCOM is an on-board UART to USB converter alongside the *Segger J-Link debugger*, connected with microcontroller pins `PA0` (RX) `PF2` (TX).

**Warning!** If the *Energy profiler* inside Simplicity Studio is used, printing to VCOM doesn't really work, use an external UART to USB converter while profiling the energy usage!

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


## Alternate locations of pins

| Location |  #0  |  #1  |  #2  |  #3  |  #4  |  #5  |  #6  |
| -------- |:----:|:----:|:----:|:----:|:----:|:----:|:----:| 
| US0_RX   | PE11 |      | PC10 | PE12 | PB08 | PC01 | PC01 |
| US0_TX   | PE10 |      |      | PE13 | PB07 | PC00 | PC00 |
| US1_RX   | PC01 |      | PD06 | PD06 | PA00 | PC02 |      |
| US1_TX   | PC00 |      | PD07 | PD07 | PF02 | PC01 |      |


## Code-example that can be used in the "while(1)" loop in "main.c
```C
/* Notified by the RX handler */
	if (dbprint_rx_data_ready)
	{
		uint32_t i;

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
		dbprint_tx_buffer[i++] = '\f'; /* Todo: this here might not be optimal */
		dbprint_tx_buffer[i] = '\0';
		dbprint_rx_data_ready = 0;

		/* Enable "RX Data Valid Interrupt" and "TX Complete Interrupt" interrupts */
		USART_IntEnable(dbpointer, USART_IEN_RXDATAV);
		USART_IntEnable(dbpointer, USART_IEN_TXC);

		/* Set TX Complete Interrupt Flag */
		USART_IntSet(dbpointer, USART_IFS_TXC);
	}
```


