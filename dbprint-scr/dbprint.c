/***************************************************************************//**
 * @file dbprint.c
 * @brief Homebrew println/printf replacement "DeBugPRINT".
 * @details Originally designed for use on the Silicion Labs Happy Gecko EFM32 board (EFM32HG322 -- TQFP48).
 * @version 2.3
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section License
 *   Some methods use code obtained from examples from Silicon Labs,
 *   these sections are licensed under the Silabs License Agreement. See the file
 *   "Silabs_License_Agreement.txt" for details. Before using this software for
 *   any purpose, you must agree to the terms of that agreement.
 *
 *   Code was obtained from examples from "https://github.com/SiliconLabs/peripheral_examples":
 *      - usart/async_polled/src/main_s0.c (version 0.0.1)
 *      - usart/async_interrupt/src/main_s0.c (version 0.0.1)
 *
 * ******************************************************************************
 *
 * @note
 *   Energy profiler seems to use VCOM (on-board UART to USB converter)somehow,
 *   change to using an external UART adapter if both the energy profiler and UART debugging
 *   are necessary at the same time!
 *   If the energy profiler was used and this define was switched, physically replug the board
 *   to make sure VCOM UART starts working again!
 *
 * ******************************************************************************
 *
 * @section Versions
 *   Please check "https://github.com/Fescron/dbprint" to find the latest version of dbprint!
 *
 *   v1.0: "define" used to jump between VCOM or other mode, itoa (<stdlib.h>) used aswell as stdio.h
 *   v1.1: Separated printInt method in a separate function for printing "int32_t" and "uint32_t" values
 *   v1.2: Added more options to the initialize method (location selection & boolean if VCOM is used)
 *   v2.0: Restructure files to be used in other projects, added a lot more documentation, added "dbAlert" and "dbClear" methods
 *   v2.1: Add interrupt functionality
 *   v2.2: Add parse functions, separated method for printing uint values in a separate one for DEC and HEX notation
 *   v2.3: Update documentation
 *
 * ******************************************************************************
 *
 * @note
 *   Debug using VCOM, no interrupts:
 *
 *   dbprint_INIT(USART1, 4, true, false);
 *
 * ******************************************************************************
 *
 * @section "C" keywords
 *
 *   Volatile
 *     The “volatile” type indicates to the compiler that the data is not normal memory,
 *     and could change at unexpected times. Hardware registers are often volatile,
 *     and so are variables which get changed in interrupts.
 *
 *   Extern
 *     Declare the global variables in headers (and use the "extern" keyword there)
 *     and actually define them in the appropriate source file.
 *
 *   Static
 *     Static variable inside a function:
 *         The variable keeps its value between invocations.
 *     Static global variable or function:
 *         The variable or function is only "seen" in the file it's declared in.
 *
 ******************************************************************************/


#include "dbprint.h"

#include <stdlib.h> /* itoa TODO: get this away from here */


/* Macro definitions that return a character */
#define TO_HEX(i) (i <= 9 ? '0' + i : 'A' - 10 + i) /* "?:" = ternary operator (return ['0' + i] if [i <= 9] = true, ['A' - 10 + i] if false) */
#define TO_DEC(i) (i <= 9 ? '0' + i : '?') /* return "?" if out of range */


/* Global variables */
USART_TypeDef* dbpointer;

/* Volatile global variables */
volatile bool dbprint_rxdata = false; /* true if there is a line of data received */
volatile char dbprint_rx_buffer[DBPRINT_BUFFER_SIZE];
volatile char dbprint_tx_buffer[DBPRINT_BUFFER_SIZE];



/**************************************************************************//**
 * @brief
 *   Initialize USARTx.
 *
 * @note
 *   Alternate Functionality Pinout:
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
 * @param[in] pointer
 *   Pointer to USARTx.
 *
 * @param[in] location
 *   Location for the pin routing.
 *
 * @param[in] vcom
 *   @li true - Isolation switch enabled by setting PA9 high so the "Virtual com port (CDC)" can be used.
 *   @li false - Isolation switch disabled on the Happy Gecko board.
 *
 * @param[in] interrupts
 *   @li true - Enable interrupt functionality.
 *   @li false - No interrupt functionality is initialized.
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

	/* Enable interrupts if necessary and print welcome string (and make two times an alert sound in the console) */
	if (interrupts)
	{
		/* Initialize USART interrupts */

		/* RX Data Valid Interrupt Enable
		 *   Set when data is available in the receive buffer. Cleared when the receive buffer is empty. */
		USART_IntEnable(dbpointer, USART_IEN_RXDATAV);

		/* TX Complete Interrupt Enable
		 *   Set when a transmission has completed and no more data is available in the transmit buffer.
		 *   Cleared when a new transmission starts. */
		USART_IntEnable(dbpointer, USART_IEN_TXC);

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

		/* Print welcome string */
		dbprintln("\a\a\r\f### UART initialized (interrupt mode) ###");

		/* Set TX Complete Interrupt Flag (transmission has completed and no more data
		* is available in the transmit buffer) */
		USART_IntSet(dbpointer, USART_IFS_TXC);
	}
	/* Print welcome string (and make two times an alert sound in the console) if not in interrupt mode */
	else {
		dbprintln("\a\a\r\f### UART initialized (no interrupts) ###");
	}
}


/**************************************************************************//**
 * @brief
 *   Sound an alert in the terminal.
 *
 * @details
 *   Print the "bell" (alert) character to USARTx.
 *****************************************************************************/
void dbAlert ()
{
	USART_Tx(dbpointer, '\a');
}


/**************************************************************************//**
 * @brief
 *   Clear the terminal.
 *
 * @details
 *   Print the "form feed" character to USARTx. Accessing old data is still
 *   possible by scrolling up in the serial port program.
 *****************************************************************************/
void dbClear ()
{
	USART_Tx(dbpointer, '\f');
}


/**************************************************************************//**
 * @brief
 *   Print a string (char array) to USARTx.
 * 
 * @note
 *   If the input is not a string (ex.: "Hello world!") but a char array,
 *   the input message (array) needs to end with NULL ('\0')!
 *
 * @param[in] message
 *   The string to print to USARTx.
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
 * @brief
 *   Print a uint32_t value in decimal notation to USARTx.
 *
 * @param[in] value
 *   The uint32_t value to print to USARTx.
 *****************************************************************************/
void dbprintUint (uint32_t value)
{
	char decchar[10]; /* Needs to be 10 */
	uint32_to_charDec(decchar, value);
	dbprint(decchar);
}


/**************************************************************************//**
 * @brief
 *   Print a uint32_t value in hexadecimal notation to USARTx.
 *
 * @param[in] value
 *   The uint32_t value to print to USARTx.
 *****************************************************************************/
void dbprintUint_hex (uint32_t value)
{
	char hexchar[9]; /* Needs to be 9 */
	uint32_to_charHex(hexchar, value, true); /* true: add spacing between eight HEX chars */
	dbprint("0x");
	dbprint(hexchar);
}


/**************************************************************************//**
 * @brief
 *   Print a int32_t value to USARTx.
 *
 * @param[in] radix
 *   @li 10 - Resulting string will be in decimal notation.
 *   @li 16 - Resulting string will be in hexadecimal notation.
 *
 * @param[in] value
 *   The int32_t value to print to USARTx.
 *****************************************************************************/
void dbprintInt (uint8_t radix, int32_t value)
{
	/* TODO: stop using itoa */
	char buffer[4];

	__itoa(value, buffer, radix); /* int char* int */

	dbprint(buffer);
}


/**************************************************************************//**
 * @brief
 *   Print a string (char array) to USARTx and go to the next line.
 *
 * @param[in] message
 *   The string to print to USARTx.
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
 * @brief
 *   Convert a uint32_t value to HEX char notation (string).
 *
 * @param[out] buf
 *   The buffer to put the resulting string in.
 *   This needs have a length of 9: "char buf[9];"!
 *
 * @param[in] value
 *   The uint32_t value to convert to a string.
 *
 * @param[in] spacing
 *   @li true - Add spacing between the eight HEX chars to make two groups of four.
 *   @li false - Don't add spacing between the eight HEX chars.
 *****************************************************************************/
void uint32_to_charHex (char *buf, uint32_t value, bool spacing)
{
	/* TODO: Maybe simplify this (based on charHex_to_uint32) */

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
 * @brief
 *   Convert a uint32_t value to DEC char notation (string)
 *
 * @param[out] buf
 *   The buffer to put the resulting string in.
 *   This needs have a length of 10: "char buf[10];"!
 *
 * @param[in] value
 *   The uint32_t value to convert to a string.
 *****************************************************************************/
void uint32_to_charDec (char *buf, uint32_t value)
{
	/* TODO: Maybe simplify this (based on charDec_to_uint32) */

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
			uint8_t length = 0;
			uint8_t lengthCounter = 0;


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


/**************************************************************************//**
 * @brief
 *   Convert a string (char array) in decimal notation to a uint32_t value.
 *
 * @note
 *   If the input is not a string (ex.: "00BA0FA1") but a char array,
 *   the input buffer (array) needs to end with NULL ('\0')!
 *
 * @param[in] buf
 *   The decimal string to convert in a uint32_t value.
 *
 * @return
 *   The resulting uint32_t value.
 *****************************************************************************/
uint32_t charDec_to_uint32 (char *buf)
{
	/* TODO: Check max amount of chars fit in uint32_t? */

	uint32_t value = 0;

	/* Loop until buffer is empty */
	while (*buf)
	{
		/* Get current character, increment afterwards */
		uint8_t byte = *buf++;

		/* Convert the ASCII (decimal) character to the representing decimal value
		 * and add it to the value (which is multiplied by 10 for each position) */
		value = (value * 10) + (byte - '0');
	}

	return (value);
}


/**************************************************************************//**
 * @brief
 *   Convert a string (char array) in hexadecimal notation to a uint32_t value.
 *
 * @note
 *   If the input is not a string (ex.: "00120561") but a char array,
 *   the input buffer (array) needs to end with NULL ('\0')!
 *
 * @param[in] buf
 *   The hexadecimal string to convert in a uint32_t value.
 *
 * @return
 *   The resulting uint32_t value.
 *****************************************************************************/
uint32_t charHex_to_uint32 (char *buf)
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

	/* TODO: Maybe fix so "0x" prefixes can be omitted? Check max amount of chars fit in uint32_t? */

	uint32_t value = 0;

	/* Loop until buffer is empty */
	while (*buf)
	{
		/* Get current character, increment afterwards */
		uint8_t byte = *buf++;

		/* Convert the hex character to the 4bit equivalent number using the ASCII table indexes */
		if (byte >= '0' && byte <= '9')byte = byte - '0';
		else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
		else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;

		/* Shift one nibble (4 bits) to make space for a new digit
		 * and add the 4 bits (ANDing with a mask, 0xF = 0b1111) */
		value = (value << 4) | (byte & 0xF);
	}

	return (value);
}


/**************************************************************************//**
 * @brief
 *   USARTx RX interrupt service routine.
 *
 * @note
 *   The "weak" definition for this method is located in "system_efm32hg.h".
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

	/* Set dbprint_rxdata when a special character is received (~ full line received) */
	if (dbprint_rx_buffer[i - 1] == '\r' || dbprint_rx_buffer[i - 1] == '\f')
	{
		dbprint_rxdata = true;
		dbprint_rx_buffer[i - 1] = '\0'; /* Overwrite CR or LF character */
		i = 0;
	}

	/* Set dbprint_rxdata when the buffer is full */
	if ( i >= DBPRINT_BUFFER_SIZE - 2 )
	{
		dbprint_rxdata = true;
		dbprint_rx_buffer[i] = '\0'; /* Do not overwrite last character */
		i = 0;
	}
}


/**************************************************************************//**
 * @brief
 *   USARTx TX interrupt service routine.
 *
 * @note
 *   The "weak" definition for this method is located in "system_efm32hg.h".
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
		 * the current item to print is not "NULL" (\0) */
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

