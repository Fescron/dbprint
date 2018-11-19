/***************************************************************************//**
 * @file dbprint.c
 * @brief Homebrew println/printf replacement "DeBugPRINT"
 * @details Originally designed for use on the Silicion Labs Happy Gecko EFM32 board (EFM32HG322 -- TQFP48)
 * @version 2.1
 * @author Brecht Van Eeckhoudt
 *
 * @note
 *   Energy profiler seems to use VCOM (on-board UART to USB converter)somehow,
 *   change to using an external UART adapter if both the energy profiler and UART debugging
 *   are necessary at the same time!
 *   If the energy profiler was used and this define was switched, physically replug the board
 *   to make sure VCOM UART starts working again!
 *
 *
 *   v1.0: "define" used to jump between VCOM or other mode, itoa (<stdlib.h>) used aswell as stdio.h
 *   v1.1: Separated printInt method in a seperate function for printing "int32_t" and "uint32_t" values,
 *   v1.2: Added more options to the initialize method (location selection & boolean if VCOM is used)
 *   v2.0: Restructure files to be used in other projects, added a lot more documentation and "dbAlert" and "dbClear" methods
 *   v2.1: Add interrupt functionality
 *
 *
 *   TODO: Optimize "CMU_ClockEnable(cmuClock_USART0, true)" clock selection
 *         Stop using itoa for int32_t conversion method
 *
 ******************************************************************************/

/*
 * Debug using VCOM, no interrupts: dbprint_INIT(USART1, 4, true, false);
 *
 */


#include "dbprint.h"

#include <stdlib.h> /* itoa */


/* Macro definitions that return a character */
#define TO_HEX(i) (i <= 9 ? '0' + i : 'A' - 10 + i) /* "?:" = ternary operator (return ['0' + i] if [i <= 9] = true, ['A' - 10 + i] if false) */
#define TO_DEC(i) (i <= 9 ? '0' + i : '?') /* return "?" if out of range */


/* Global variables */
USART_TypeDef* dbpointer;

/* Volatile global variables */
volatile bool dbprint_rx_data_ready = 0;
volatile char dbprint_rx_buffer[DBPRINT_BUFFER_SIZE];
volatile char dbprint_tx_buffer[DBPRINT_BUFFER_SIZE];


/*
 * "static" variable inside a function:     keeps its value between invocations.
 * "static global" variable or a function:  only "seen" in the file it's declared in
 */



/**************************************************************************//**
 * @brief Initialize USARTx
 *
 * @note Alternate Functionality Pinout:
 *
 *      Location |  #0  |  #1  |  #2  |  #3  |  #4  |  #5  |  #6  |     ||===================||
 *      -----------------------------------------------------------     || baudrate = 115200 ||
 *      US0_RX   | PE11 |      | PC10 | PE12 | PB08 | PC01 | PC01 |     || 8 databits        ||
 *      US0_TX   | PE10 |      |      | PE13 | PB07 | PC00 | PC00 |     || 1 stopbit         ||
 *      -----------------------------------------------------------     || no parity         ||
 *      US1_RX   | PC01 |      | PD06 | PD06 | PA00 | PC02 |      |     ||===================||
 *      US1_TX   | PC00 |      | PD07 | PD07 | PF02 | PC01 |      |
 *
 *      VCOM: USART1 #4 (USART0 not possible) RX: PA0 -- TX: PF2
 *
 * @param pointer USARTx pointer
 * @param location Location for the pin routing
 * @param vcom If true: route TX and RX to "Virtual com port (CDC)" on Happy Ghecko board (PA9 is also set high to enable the isolation switch)
 * @param vcom If true: enable interrupt functionality
 *****************************************************************************/
void dbprint_INIT (USART_TypeDef* pointer, uint8_t location, bool vcom, bool interrupts)
{
	/* Store the pointer in the global variable */
	dbpointer = pointer;

	/*
	 * USART_INITASYNC_DEFAULT:
	 * config.enable = usartEnable			// Specifies whether TX and/or RX is enabled when initialization is completed (Enable RX/TX when initialization is complete).
	 * config.refFreq = 0					// USART/UART reference clock assumed when configuring baud rate setup (0 = Use current configured reference clock for configuring baud rate).
	 * config.baudrate = 115200				// Desired baudrate (115200 bits/s).
	 * config.oversampling = usartOVS16		// Oversampling used (16x oversampling).
	 * config.databits = usartDatabits8		// Number of data bits in frame (8 data bits).
	 * config.parity = usartNoParity		// Parity mode to use (no parity).
	 * config.stopbits = usartStopbits1		// Number of stop bits to use (1 stop bit).
	 * config.mvdis = false					// Majority Vote Disable for 16x, 8x and 6x oversampling modes (Do not disable majority vote).
	 * config.prsRxEnable = false			// Enable USART Rx via PRS (Not USART PRS input mode).
	 * config.prsRxCh = 0					// Select PRS channel for USART Rx. (Only valid if prsRxEnable is true - PRS channel 0).
	 * config.autoCsEnable = false			// Auto CS enabling (Auto CS functionality enable/disable switch - disabled).
	 */

	USART_InitAsync_TypeDef config = USART_INITASYNC_DEFAULT;

	/* Enable oscillator to GPIO*/
	CMU_ClockEnable(cmuClock_GPIO, true);


	/* Enable oscillator to USARTx modules
	 * TODO: Optimize this!
	 */
	if (dbpointer == USART0) {
		CMU_ClockEnable(cmuClock_USART0, true);
	}
	else if (dbpointer == USART1) {
		CMU_ClockEnable(cmuClock_USART1, true);
	}


	/* Set PA9 (EFM_BC_EN) high if necessary to enable the isolation switch */
	if (vcom)
	{
		GPIO_PinModeSet(gpioPortA, 9, gpioModePushPull, 1);
		GPIO_PinOutSet(gpioPortA, 9);
	}


	/* Set pin modes for UART TX and RX pins */
	if (dbpointer == USART0) {
		switch (location)
		{
			case 0:
				GPIO_PinModeSet(gpioPortE, 11, gpioModeInput, 0);	/* RX */
				GPIO_PinModeSet(gpioPortE, 10, gpioModePushPull, 1);/* TX */
				break;
			case 2:
				GPIO_PinModeSet(gpioPortC, 10, gpioModeInput, 0);	/* RX */
				/* No TX pin in this mode */
				break;
			case 3:
				GPIO_PinModeSet(gpioPortE, 12, gpioModeInput, 0);	/* RX */
				GPIO_PinModeSet(gpioPortE, 13, gpioModePushPull, 1);/* TX */
				break;
			case 4:
				GPIO_PinModeSet(gpioPortB, 8, gpioModeInput, 0);	/* RX */
				GPIO_PinModeSet(gpioPortB, 7, gpioModePushPull, 1); /* TX */
				break;
			case 5:
			case 6:
				GPIO_PinModeSet(gpioPortC, 1, gpioModeInput, 0);	/* RX */
				GPIO_PinModeSet(gpioPortC, 0, gpioModePushPull, 1); /* TX */
				break;
			/* default: */
				/* No default */
		}
	}
	else if (dbpointer == USART1) {
		switch (location)
		{
			case 0:
				GPIO_PinModeSet(gpioPortC, 1, gpioModeInput, 0);	/* RX */
				GPIO_PinModeSet(gpioPortC, 0, gpioModePushPull, 1); /* TX */
				break;
			case 2:
			case 3:
				GPIO_PinModeSet(gpioPortD, 6, gpioModeInput, 0);	/* RX */
				GPIO_PinModeSet(gpioPortD, 7, gpioModePushPull, 1); /* TX */
				break;
			case 4:
				GPIO_PinModeSet(gpioPortA, 0, gpioModeInput, 0);	/* RX */
				GPIO_PinModeSet(gpioPortF, 2, gpioModePushPull, 1); /* TX */
				break;
			case 5:
				GPIO_PinModeSet(gpioPortC, 2, gpioModeInput, 0);	/* RX */
				GPIO_PinModeSet(gpioPortC, 1, gpioModePushPull, 1); /* TX */
				break;
			/* default: */
				/* No default */
		}
	}


	/* Initialize USART asynchronous mode */
	USART_InitAsync(dbpointer, &config);

	/* Route pins */
	switch (location)
	{
		case 0:
			dbpointer->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC0;
			break;
		case 1:
			dbpointer->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC1;
			break;
		case 2:
			dbpointer->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC2;
			break;
		case 3:
			dbpointer->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC3;
			break;
		case 4:
			dbpointer->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC4;
			break;
		case 5:
			dbpointer->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC5;
			break;
		case 6:
			dbpointer->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC6;
			break;
		default:
			dbpointer->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_DEFAULT;
	}

	/* Enable interrupts if necessary and print "welcome" string */
	if (interrupts)
	{
		/* Initialize USART interrupts */
		USART_IntEnable(dbpointer, USART_IEN_RXDATAV); /* RX Data Valid Interrupt Enable */
		USART_IntEnable(dbpointer, USART_IEN_TXC);     /* TX Complete Interrupt Enable */

		if (dbpointer == USART0)
		{
			/* Enable USART interrupts */
			NVIC_EnableIRQ(USART0_RX_IRQn);
			NVIC_EnableIRQ(USART0_TX_IRQn);
		}
		else if (dbpointer == USART1)
		{
			/* Enable USART interrupts */
			NVIC_EnableIRQ(USART1_RX_IRQn);
			NVIC_EnableIRQ(USART1_TX_IRQn);
		}

		/* Print welcome string using interrupts */
		char welcome[] = "\r\f### UART initialized (interrupt mode) ###";

		for (uint8_t i = 0 ; welcome[i] != 0; i++)
		{
			/* Put the character on UART using interrupts (IRQ handler is executed) */
			dbprint_tx_buffer[i] = welcome[i];
		}

		/* Set TX Complete Interrupt Flag */
		USART_IntSet(dbpointer, USART_IFS_TXC);
	}
	/* Print welcome string */
	else {
		dbClear();

		/* Before clear method: "\r\n\n### UART initialized ###"
		 * TODO: Does \f also fix were \n and \r should be necessary?
		 */
		dbprintln("### UART initialized (no interrupts) ###");
	}
}


/**************************************************************************//**
 * @brief Print the BELL character to USARTx to sound an ALERT
 *****************************************************************************/
void dbAlert ()
{
	USART_Tx(dbpointer, '\a');
}


/**************************************************************************//**
 * @brief Clear the terminal
 *****************************************************************************/
void dbClear ()
{
	/* form feed (flush terminal, accessing old data by scrolling up is possible) */
	USART_Tx(dbpointer, '\f');
}


/**************************************************************************//**
 * @brief Print a string (char array) to USARTx
 *
 * @param message The message to display
 *****************************************************************************/
void dbprint (char *message)
{
	/* "message[i] != 0" makes "uint32_t length = strlen(message)"
	 * not necessary (given string MUST be terminated by NULL for this to work) */
	for (uint32_t i = 0; message[i] != 0; i++)
	{
		USART_Tx(dbpointer, message[i]);
	}
}


/**************************************************************************//**
 * @brief Print a uint32_t to USARTx
 *
 * @param radix 10 for decimal, 16 for hexadecimal
 * @param value The value to display
 *****************************************************************************/
void dbprintUint (uint8_t radix, uint32_t value)
{
	/* Decimal notation */
	if (radix == 10)
	{
		/* Convert "value" to decimal characters and put them in the buffer (decchar) */
		char decchar[10]; /* Needs to be 10 */
		uint32_to_charDec(decchar, value);
		dbprint(decchar);
	}

	/* Hexadecimal notation */
	else if (radix == 16)
	{
		/* Convert "value" to hexadecimal characters and put them in the buffer (hexchar) */
		char hexchar[9]; /* Needs to be 9 */
		uint32_to_charHex(hexchar, value, true); /* true: add spacing between eight HEX chars */
		dbprint("0x");
		dbprint(hexchar);
	}
}


/**************************************************************************//**
 * @brief Print a int32_t to USARTx
 *
 * @param radix 10 for decimal, 16 for hexadecimal
 * @param value The value to display
 *****************************************************************************/
void dbprintInt (uint8_t radix, int32_t value)
{
	/* TODO: stop using itoa */
	char buffer[4];

	__itoa(value, buffer, radix); /* int char* int */

	dbprint(buffer);
}


/**************************************************************************//**
 * @brief Print a string (char array) to USARTx and go to the next line
 *
 * @param message The message to display
 *****************************************************************************/
void dbprintln (char *message)
{
	dbprint(message);

	/* Carriage return */
	USART_Tx(dbpointer, '\r');

	/* Line feed (new line) */
	USART_Tx(dbpointer, '\n');
}


/**************************************************************************//**
 * @brief USARTx RX interrupt service routine
 * @note The "weak" definition for this method is in "system_efm32hg.h"
 *****************************************************************************/
void USART1_RX_IRQHandler(void)
{
	/* "static" so it keeps its value between invocations */
	static uint32_t i = 0;

	/* Get and clear the pending USART interrupt flags */
	uint32_t flags = USART_IntGet(dbpointer);
	USART_IntClear(dbpointer, flags);

	/* Store incoming data into dbprint_rx_buffer */
	dbprint_rx_buffer[i++] = USART_Rx(dbpointer);

	/* Set dbprint_rx_data_ready when a special character is received (~ full line received) */
	if (dbprint_rx_buffer[i - 1] == '\r' || dbprint_rx_buffer[i - 1] == '\f')
	{
		dbprint_rx_data_ready = 1;
		dbprint_rx_buffer[i - 1] = '\0'; /* Overwrite CR or LF character */
		i = 0;
	}

	/* Set dbprint_rx_data_ready when the buffer is full */
	if ( i >= DBPRINT_BUFFER_SIZE - 2 )
	{
		dbprint_rx_data_ready = 1;
		dbprint_rx_buffer[i] = '\0'; /* Do not overwrite last character */
		i = 0;
	}
}


/**************************************************************************//**
 * @brief USARTx TX interrupt service routine
 * @note The "weak" definition for this method is in "system_efm32hg.h"
 *****************************************************************************/
void USART1_TX_IRQHandler(void)
{
	/* "static" so it keeps its value between invocations */
	static uint32_t i = 0;

	/* Get and clear the pending USART interrupt flags */
	uint32_t flags = USART_IntGet(dbpointer);
	USART_IntClear(dbpointer, flags);

	/* Mask flags AND "TX Complete Interrupt Flag" */
	if (flags & USART_IF_TXC)
	{
		/* Index is smaller than the maximum buffer size and
		 * the current item to print is not "NULL" (\0)
		 */
		if (i < DBPRINT_BUFFER_SIZE && dbprint_tx_buffer[i] != '\0')
		{
			/* Transmit byte at current index and increment index */
			USART_Tx(dbpointer, dbprint_tx_buffer[i++]);
		}
		else
		{
			i = 0; /* No more data to send */
		}
	}
}


/**************************************************************************//**
 * @brief Convert uint32_t to HEX char notation (string)
 *
 * @param buf The buffer (needs to be: "char buf[9];")
 * @param value The uint32_t value
 * @param spacing True if there needs to be added spacing between the eight HEX chars
 *****************************************************************************/
void uint32_to_charHex (char *buf, uint32_t value, bool spacing)
{
	/*
	 * 1 nibble = 4 bits (0b1111      = 0xF )
	 * 1 byte   = 8 bits (0b1111 1111 = 0xFF)
	 *
	 * uint8_t  ~ unsigned char		1 byte  (0 - 255 or 0xFF)
	 * uint16_t ~ unsigned short	2 bytes (0 - 65535 or 0xFFFF)
	 * uint32_t ~ unsigned int		4 bytes (0 - 4294967295 or 0xFFFFFFFF)
	 *
	 */

	/* Checking just in case */
	if (value <= 0xFFFFFFFF)
	{
		/* 4 nibble HEX representation */
		if (value <= 0xFFFF)
		{
			/* Only get necessary nibble by ANDing with a mask and
			 * shifting one nibble (4 bits) per position */
			buf[0] = TO_HEX(((value & 0xF000) >> 12));
			buf[1] = TO_HEX(((value & 0x0F00) >> 8 ));
			buf[2] = TO_HEX(((value & 0x00F0) >> 4 ));
			buf[3] = TO_HEX( (value & 0x000F)       );
			buf[4] = '\0'; /* NULL termination character */
		}

		/* 8 nibble HEX representation */
		else
		{
			/* Only get necessary nibble by ANDing with a mask and
			 * shifting one nibble (4 bits) per position */
			buf[0] = TO_HEX(((value & 0xF0000000) >> 28));
			buf[1] = TO_HEX(((value & 0x0F000000) >> 24));
			buf[2] = TO_HEX(((value & 0x00F00000) >> 20));
			buf[3] = TO_HEX(((value & 0x000F0000) >> 16));

			/* Add spacing if necessary */
			if (spacing)
			{
				buf[4] = ' '; /* Spacing */
				buf[5] = TO_HEX(((value & 0x0000F000) >> 12));
				buf[6] = TO_HEX(((value & 0x00000F00) >> 8 ));
				buf[7] = TO_HEX(((value & 0x000000F0) >> 4 ));
				buf[8] = TO_HEX( (value & 0x0000000F)       );
				buf[9] = '\0'; /* NULL termination character */
			}
			else
			{
				buf[4] = TO_HEX(((value & 0x0000F000) >> 12));
				buf[5] = TO_HEX(((value & 0x00000F00) >> 8 ));
				buf[6] = TO_HEX(((value & 0x000000F0) >> 4 ));
				buf[7] = TO_HEX( (value & 0x0000000F)       );
				buf[8] = '\0'; /* NULL termination character */
			}

		}
	}
}


/**************************************************************************//**
 * @brief Convert uint32_t to DEC char notation (string)
 *
 * @param buf The buffer (needs to be: "char buf[10];")
 * @param value The uint32_t value
 *****************************************************************************/
void uint32_to_charDec (char *buf, uint32_t value)
{
	/* Checking just in case */
	if (value <= 0xFFFFFFFF)
	{
		if (value == 0)
		{
			buf[0] = '0';
			buf[1] = '\0'; /* NULL termination character */
		}
		else
		{
			/* MAX uint32_t value = FFFFFFFFh = 4294967295d (10 chars in dec) */
			char backwardsBuf[10];

			uint32_t calcval = value;
			uint8_t length, lengthCounter = 0;


			/* Loop until the value is zero (separate characters 0-9) and calculate length */
			while (calcval) {
				uint32_t rem = calcval % 10;
				backwardsBuf[length] = TO_DEC(rem); /* Convert to ASCII character */
				length++;

				calcval = calcval - rem;
				calcval = calcval / 10;
			}

			/* Backwards counter */
			lengthCounter = length;

			/* Reverse the characters in the buffer for the final string */
			for (uint8_t i = 0; i < length; i++)
			{
				buf[i] = backwardsBuf[lengthCounter-1];
				lengthCounter--;
			}

			/* Add NULL termination character */
			buf[length] = '\0';
		}
	}
}


