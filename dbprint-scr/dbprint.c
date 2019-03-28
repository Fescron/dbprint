/***************************************************************************//**
 * @file dbprint.c
 * @brief Homebrew println/printf replacement "DeBugPRINT".
 * @details Originally designed for use on the Silicion Labs Happy Gecko EFM32 board (EFM32HG322 -- TQFP48).
 * @version 3.8
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section License
 *
 *   Some methods use code obtained from examples from Silicon Labs,
 *   these sections are licensed under the Silabs License Agreement. See the file
 *   "Silabs_License_Agreement.txt" for details. Before using this software for
 *   any purpose, you must agree to the terms of that agreement.
 *
 *   Code was obtained from examples from "https://github.com/SiliconLabs/peripheral_examples":
 *      - usart/async_polled/src/main_s0.c    (version 0.0.1)
 *      - usart/async_interrupt/src/main_s0.c (version 0.0.1)
 *
 * ******************************************************************************
 *
 * @note
 *   The Energy profiler in Simplicity Studio seems to use VCOM (on-board UART to USB converter)somehow,
 *   change to using an external UART adapter if both the energy profiler and UART debugging
 *   are necessary at the same time!
 *   If the energy profiler was used and the code functionality was switched, physically replug the board
 *   to make sure VCOM UART starts working again!
 *
 * ******************************************************************************
 *
 * @section Versions
 *
 *   Please check "https://github.com/Fescron/dbprint" to find the latest version of dbprint!
 *
 *   v1.0: "define" used to jump between VCOM or other mode, itoa (<stdlib.h>) used aswell as stdio.h
 *   v1.1: Separated printInt method in a separate function for printing "int32_t" and "uint32_t" values.
 *   v1.2: Added more options to the initialize method (location selection & boolean if VCOM is used).
 *   v2.0: Restructure files to be used in other projects, added a lot more documentation, added "dbAlert" and "dbClear" methods.
 *   v2.1: Add interrupt functionality.
 *   v2.2: Add parse functions, separated method for printing uint values in a separate one for DEC and HEX notation.
 *   v2.3: Updated documentation.
 *   v2.4: Fix notes.
 *   v2.5: Separated method for printing int values in a separate one for DEC and HEX notation.
 *   v2.6: Stop using itoa (<stdlib.h>) in all methods.
 *   v3.0: Simplify number printing, stop using separate methods for uint and int values.
 *   v3.1: Remove useless if... check.
 *   v3.2: Add the ability to print text in a color.
 *   v3.3: Add info, warning and critical error printing methods.
 *   v3.4: Add printInt(_hex) methods that directly go to a new line.
 *   v3.5: Add USART0 IRQ handlers.
 *   v3.6: Add the ability to print (u)int values as INFO, WARN or CRIT lines.
 *   v3.7: Add separate "_hex" methods for dbinfo/warn/critInt instead of a boolean to select (hexa)decimal notation.
 *   v3.8: Add ReadChar-Int-Line methods.
 *
 *   TODO (maybe):
 *     - Separate back-end <-> MCU specific code?
 *     - Use getters and setters instead of "extern" for the interrupt buffers?
 *         -> Not safe if multiple ISR's are using the same "extern"!
 *         -> Use "atomic" stuff when an action has to be performed before other interrupts can be called?
 *         -> CORE_ENTER_ATOMIC() ? ~ disable certain interrupts
 *     - Does dbprint.c code still compiles when #define DEBUGGING is gone?
 *
 *     - GitHub: Add "debugging.h" file as example.
 *     - Github: Explain other method of importing?
 *
 * ******************************************************************************
 *
 *  @section Alternate Functionality Pinout
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
 * ******************************************************************************
 *
 * @section Debug using VCOM (no interrupt functionality)
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
 * ******************************************************************************
 *
 * @section Bits, bytes, nibbles and unsigned/signed integer value ranges
 *
 *     - 1 nibble = 4 bits (0b1111      = 0xF )
 *     - 1 byte   = 8 bits (0b1111 1111 = 0xFF)
 *
 *     - uint8_t  ~ unsigned char  = 1 byte  (0 > 255           or 0xFF)
 *     - uint16_t ~ unsigned short = 2 bytes (0 > 65 535        or 0xFFFF)
 *     - uint32_t ~ unsigned int   = 4 bytes (0 > 4 294 967 295 or 0xFFFFFFFF)
 *
 *     - int8_t   ~ signed char    = 1 byte  (-128 > 127)
 *     - int16_t  ~ signed short   = 2 bytes (-32 768 > 32 767)
 *     - int32_t  ~ signed int     = 4 bytes (-2 147 483 648 > 2 147 483 647)
 *
 ******************************************************************************/


#include "dbprint.h"


/* Macro definitions that return a character when given a value */
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
	 *   config.enable = usartEnable       // Specifies whether TX and/or RX is enabled when initialization is completed
	 *                                     // (Enable RX/TX when initialization is complete).
	 *   config.refFreq = 0                // USART/UART reference clock assumed when configuring baud rate setup
	 *                                     // (0 = Use current configured reference clock for configuring baud rate).
	 *   config.baudrate = 115200          // Desired baudrate (115200 bits/s).
	 *   config.oversampling = usartOVS16  // Oversampling used (16x oversampling).
	 *   config.databits = usartDatabits8  // Number of data bits in frame (8 data bits).
	 *   config.parity = usartNoParity     // Parity mode to use (no parity).
	 *   config.stopbits = usartStopbits1  // Number of stop bits to use (1 stop bit).
	 *   config.mvdis = false              // Majority Vote Disable for 16x, 8x and 6x oversampling modes (Do not disable majority vote).
	 *   config.prsRxEnable = false        // Enable USART Rx via PRS (Not USART PRS input mode).
	 *   config.prsRxCh = 0                // Select PRS channel for USART Rx. (Only valid if prsRxEnable is true - PRS channel 0).
	 *   config.autoCsEnable = false       // Auto CS enabling (Auto CS functionality enable/disable switch - disabled).
	 */

	USART_InitAsync_TypeDef config = USART_INITASYNC_DEFAULT;

	/* Enable oscillator to GPIO*/
	CMU_ClockEnable(cmuClock_GPIO, true);


	/* Enable oscillator to USARTx modules */
	if (dbpointer == USART0)
	{
		CMU_ClockEnable(cmuClock_USART0, true);
	}
	else if (dbpointer == USART1)
	{
		CMU_ClockEnable(cmuClock_USART1, true);
	}


	/* Set PA9 (EFM_BC_EN) high if necessary to enable the isolation switch */
	if (vcom)
	{
		GPIO_PinModeSet(gpioPortA, 9, gpioModePushPull, 1);
		GPIO_PinOutSet(gpioPortA, 9);
	}


	/* Set pin modes for UART TX and RX pins */
	if (dbpointer == USART0)
	{
		switch (location)
		{
			case 0:
				GPIO_PinModeSet(gpioPortE, 11, gpioModeInput, 0);    /* RX */
				GPIO_PinModeSet(gpioPortE, 10, gpioModePushPull, 1); /* TX */
				break;
			case 2:
				GPIO_PinModeSet(gpioPortC, 10, gpioModeInput, 0);    /* RX */
				/* No TX pin in this mode */
				break;
			case 3:
				GPIO_PinModeSet(gpioPortE, 12, gpioModeInput, 0);    /* RX */
				GPIO_PinModeSet(gpioPortE, 13, gpioModePushPull, 1); /* TX */
				break;
			case 4:
				GPIO_PinModeSet(gpioPortB, 8, gpioModeInput, 0);     /* RX */
				GPIO_PinModeSet(gpioPortB, 7, gpioModePushPull, 1);  /* TX */
				break;
			case 5:
			case 6:
				GPIO_PinModeSet(gpioPortC, 1, gpioModeInput, 0);     /* RX */
				GPIO_PinModeSet(gpioPortC, 0, gpioModePushPull, 1);  /* TX */
				break;
			/* default: */
				/* No default */
		}
	}
	else if (dbpointer == USART1)
	{
		switch (location)
		{
			case 0:
				GPIO_PinModeSet(gpioPortC, 1, gpioModeInput, 0);     /* RX */
				GPIO_PinModeSet(gpioPortC, 0, gpioModePushPull, 1);  /* TX */
				break;
			case 2:
			case 3:
				GPIO_PinModeSet(gpioPortD, 6, gpioModeInput, 0);     /* RX */
				GPIO_PinModeSet(gpioPortD, 7, gpioModePushPull, 1);  /* TX */
				break;
			case 4:
				GPIO_PinModeSet(gpioPortA, 0, gpioModeInput, 0);     /* RX */
				GPIO_PinModeSet(gpioPortF, 2, gpioModePushPull, 1);  /* TX */
				break;
			case 5:
				GPIO_PinModeSet(gpioPortC, 2, gpioModeInput, 0);     /* RX */
				GPIO_PinModeSet(gpioPortC, 1, gpioModePushPull, 1);  /* TX */
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

	/* Enable interrupts if necessary and print welcome string (and make an alert sound in the console) */
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
		dbprintln("\a\r\f### UART initialized (interrupt mode) ###");
		dbinfo("This is an info message.");
		dbwarn("This is a warning message.");
		dbcrit("This is a critical error message.");
		dbprintln("###  Start executing programmed code  ###\n");

		/* Set TX Complete Interrupt Flag (transmission has completed and no more data
		* is available in the transmit buffer) */
		USART_IntSet(dbpointer, USART_IFS_TXC);
	}
	/* Print welcome string (and make an alert sound in the console) if not in interrupt mode */
	else
	{
		dbprintln("\a\r\f### UART initialized (no interrupts) ###");
		dbinfo("This is an info message.");
		dbwarn("This is a warning message.");
		dbcrit("This is a critical error message.");
		dbprintln("### Start executing programmed code  ###\n");
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
 *   Print a string (char array) to USARTx and go to the next line.
 *
 * @note
 *   If the input is not a string (ex.: "Hello world!") but a char array,
 *   the input message (array) needs to end with NULL ('\0')!
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
 *   Print a string (char array) to USARTx in a given color.
 *
 * @note
 *   If the input is not a string (ex.: "Hello world!") but a char array,
 *   the input message (array) needs to end with NULL ('\0')!
 *
 * @param[in] message
 *   The string to print to USARTx.
 *
 * @param[in] color
 *   The color to print the text in.
 *   @li 0 - Reset color
 *   @li 1 - Red
 *   @li 2 - Green
 *   @li 3 - Blue
 *   @li 4 - Cyan
 *   @li 5 - Magenta
 *   @li 6 - Yellow
 *****************************************************************************/
void dbprint_color (char *message, uint8_t color)
{
	switch (color)
	{
		case 0:
			dbprint(COLOR_RESET);
			dbprint(message);
			break;
		case 1:
			dbprint(COLOR_RED);
			dbprint(message);
			dbprint(COLOR_RESET);
			break;
		case 2:
			dbprint(COLOR_GREEN);
			dbprint(message);
			dbprint(COLOR_RESET);
			break;
		case 3:
			dbprint(COLOR_BLUE);
			dbprint(message);
			dbprint(COLOR_RESET);
			break;
		case 4:
			dbprint(COLOR_CYAN);
			dbprint(message);
			dbprint(COLOR_RESET);
			break;
		case 5:
			dbprint(COLOR_MAGENTA);
			dbprint(message);
			dbprint(COLOR_RESET);
			break;
		case 6:
			dbprint(COLOR_YELLOW);
			dbprint(message);
			dbprint(COLOR_RESET);
			break;
		default:
			dbprint(COLOR_RESET);
			dbprint(message);
	}
}


/**************************************************************************//**
 * @brief
 *   Print a string (char array) to USARTx in a given color and go to the next line.
 *
 * @note
 *   If the input is not a string (ex.: "Hello world!") but a char array,
 *   the input message (array) needs to end with NULL ('\0')!
 *
 * @param[in] message
 *   The string to print to USARTx.
 *
 * @param[in] color
 *   The color to print the text in.
 *   @li 0 - Reset color
 *   @li 1 - Red
 *   @li 2 - Green
 *   @li 3 - Blue
 *   @li 4 - Cyan
 *   @li 5 - Magenta
 *   @li 6 - Yellow
 *****************************************************************************/
void dbprintln_color (char *message, uint8_t color)
{
	dbprint_color(message, color);

	/* Carriage return */
	USART_Tx(dbpointer, '\r');

	/* Line feed (new line) */
	USART_Tx(dbpointer, '\n');
}


/**************************************************************************//**
 * @brief
 *   Print an info string (char array) to USARTx and go to the next line.
 *
 * @note
 *   If the input is not a string (ex.: "Hello world!") but a char array,
 *   the input message (array) needs to end with NULL ('\0')!
 *
 * @param[in] message
 *   The string to print to USARTx.
 *****************************************************************************/
void dbinfo (char *message)
{
	dbprint("INFO: ");
	dbprintln(message);
}


/**************************************************************************//**
 * @brief
 *   Print a warning string (char array) in yellow to USARTx and go to the next line.
 *
 * @note
 *   If the input is not a string (ex.: "Hello world!") but a char array,
 *   the input message (array) needs to end with NULL ('\0')!
 *
 * @param[in] message
 *   The string to print to USARTx.
 *****************************************************************************/
void dbwarn (char *message)
{
	dbprint_color("WARN: ", 6);
	dbprintln_color(message, 6);
}


/**************************************************************************//**
 * @brief
 *   Print a critical error string (char array) in red to USARTx and go to the next line.
 *
 * @note
 *   If the input is not a string (ex.: "Hello world!") but a char array,
 *   the input message (array) needs to end with NULL ('\0')!
 *
 * @param[in] message
 *   The string to print to USARTx.
 *****************************************************************************/
void dbcrit (char *message)
{
	dbprint_color("CRIT: ", 1);
	dbprintln_color(message, 1);
}


/**************************************************************************//**
 * @brief
 *   Print a value surrounded by two strings (char array) to USARTx
 *   with "INFO: " added in front in decimal notation and go to the next line.
 *
 * @note
 *   If the input is not a string (ex.: "Hello world!") but a char array,
 *   the input message (array) needs to end with NULL ('\0')!
 *
 * @param[in] message1
 *   The first part of the string to print to USARTx.
 *
 * @param[in] value
 *   The value to print between the two string parts.
 *
 * @param[in] message2
 *   The second part of the string to print to USARTx.
 *****************************************************************************/
void dbinfoInt (char *message1, int32_t value, char *message2)
{
	dbprint("INFO: ");
	dbprint(message1);
	dbprintInt(value);
	dbprintln(message2);
}


/**************************************************************************//**
 * @brief
 *   Print a value surrounded by two strings (char array) to USARTx
 *   with "WARN: " added in front in decimal notation and go to the next line.
 *   The value is in the color white, the rest is yellow.
 *
 * @note
 *   If the input is not a string (ex.: "Hello world!") but a char array,
 *   the input message (array) needs to end with NULL ('\0')!
 *
 * @param[in] message1
 *   The first part of the string to print to USARTx.
 *
 * @param[in] value
 *   The value to print between the two string parts.
 *
 * @param[in] message2
 *   The second part of the string to print to USARTx.
 *****************************************************************************/
void dbwarnInt (char *message1, int32_t value, char *message2)
{
	dbprint_color("WARN: ", 6);
	dbprint_color(message1, 6);
	dbprintInt(value);
	dbprintln_color(message2, 6);
}


/**************************************************************************//**
 * @brief
 *   Print a value surrounded by two strings (char array) to USARTx
 *   with "CRIT: " added in front in decimal notation and go to the next line.
 *   The value is in the color white, the rest is red.
 *
 * @note
 *   If the input is not a string (ex.: "Hello world!") but a char array,
 *   the input message (array) needs to end with NULL ('\0')!
 *
 * @param[in] message1
 *   The first part of the string to print to USARTx.
 *
 * @param[in] value
 *   The value to print between the two string parts.
 *
 * @param[in] message2
 *   The second part of the string to print to USARTx.
 *****************************************************************************/
void dbcritInt (char *message1, int32_t value, char *message2)
{
	dbprint_color("CRIT: ", 1);
	dbprint_color(message1, 1);
	dbprintInt(value);
	dbprintln_color(message2, 1);
}


/**************************************************************************//**
 * @brief
 *   Print a value surrounded by two strings (char array) to USARTx
 *   with "INFO: " added in front in hexadecimal notation and go to the next line.
 *
 * @note
 *   If the input is not a string (ex.: "Hello world!") but a char array,
 *   the input message (array) needs to end with NULL ('\0')!
 *
 * @param[in] message1
 *   The first part of the string to print to USARTx.
 *
 * @param[in] value
 *   The value to print between the two string parts.
 *
 * @param[in] message2
 *   The second part of the string to print to USARTx.
 *****************************************************************************/
void dbinfoInt_hex (char *message1, int32_t value, char *message2)
{
	dbprint("INFO: ");
	dbprint(message1);
	dbprintInt_hex(value);
	dbprintln(message2);
}


/**************************************************************************//**
 * @brief
 *   Print a value surrounded by two strings (char array) to USARTx
 *   with "WARN: " added in front in hexadecimal notation and go to the next line.
 *   The value is in the color white, the rest is yellow.
 *
 * @note
 *   If the input is not a string (ex.: "Hello world!") but a char array,
 *   the input message (array) needs to end with NULL ('\0')!
 *
 * @param[in] message1
 *   The first part of the string to print to USARTx.
 *
 * @param[in] value
 *   The value to print between the two string parts.
 *
 * @param[in] message2
 *   The second part of the string to print to USARTx.
 *****************************************************************************/
void dbwarnInt_hex (char *message1, int32_t value, char *message2)
{
	dbprint_color("WARN: ", 6);
	dbprint_color(message1, 6);
	dbprintInt_hex(value);
	dbprintln_color(message2, 6);
}


/**************************************************************************//**
 * @brief
 *   Print a value surrounded by two strings (char array) to USARTx
 *   with "CRIT: " added in front in hexadecimal notation and go to the next line.
 *   The value is in the color white, the rest is red.
 *
 * @note
 *   If the input is not a string (ex.: "Hello world!") but a char array,
 *   the input message (array) needs to end with NULL ('\0')!
 *
 * @param[in] message1
 *   The first part of the string to print to USARTx.
 *
 * @param[in] value
 *   The value to print between the two string parts.
 *
 * @param[in] message2
 *   The second part of the string to print to USARTx.
 *****************************************************************************/
void dbcritInt_hex (char *message1, int32_t value, char *message2)
{
	dbprint_color("CRIT: ", 1);
	dbprint_color(message1, 1);
	dbprintInt_hex(value);
	dbprintln_color(message2, 1);
}


/**************************************************************************//**
 * @brief
 *   Print a number in decimal notation to USARTx.
 *
 * @param[in] value
 *   The number to print to USARTx.
 *   This can be of type "uint32_t" or "int32_t".
 *****************************************************************************/
void dbprintInt (int32_t value)
{
	/* Buffer to put the char array in (Needs to be 10) */
	char decchar[10];

	/* Convert a negative number to a positive one and print the "-" */
	if (value < 0)
	{
		/* Negative of value = flip all bits, +1
		 *   bitwise logic: "~" = "NOT" */
		uint32_t negativeValue = (~value) + 1;

		dbprint("-");

		/* Convert the value */
		uint32_to_charDec(decchar, negativeValue);
	}
	else
	{
		/* Convert the value */
		uint32_to_charDec(decchar, value);
	}

	/* Print the buffer */
	dbprint(decchar);
}


/**************************************************************************//**
 * @brief
 *   Print a number in decimal notation to USARTx and go to the next line.
 *
 * @param[in] value
 *   The number to print to USARTx.
 *   This can be of type "uint32_t" or "int32_t".
 *****************************************************************************/
void dbprintlnInt (int32_t value)
{
	dbprintInt(value);

	/* Carriage return */
	USART_Tx(dbpointer, '\r');

	/* Line feed (new line) */
	USART_Tx(dbpointer, '\n');
}


/**************************************************************************//**
 * @brief
 *   Print a number in hexadecimal notation to USARTx.
 *
 * @param[in] value
 *   The number to print to USARTx.
 *   This can be of type "uint32_t" or "int32_t".
 *****************************************************************************/
void dbprintInt_hex (int32_t value)
{
	char hexchar[9]; /* Needs to be 9 */
	uint32_to_charHex(hexchar, value, true); /* true: add spacing between eight HEX chars */
	dbprint("0x");
	dbprint(hexchar);
}


/**************************************************************************//**
 * @brief
 *   Print a number in hexadecimal notation to USARTx and go to the next line.
 *
 * @param[in] value
 *   The number to print to USARTx.
 *   This can be of type "uint32_t" or "int32_t".
 *****************************************************************************/
void dbprintlnInt_hex (int32_t value)
{
	dbprintInt_hex(value);

	/* Carriage return */
	USART_Tx(dbpointer, '\r');

	/* Line feed (new line) */
	USART_Tx(dbpointer, '\n');
}


/**************************************************************************//**
 * @brief
 *   Read a character from USARTx.
 *
 * @note
 *   To read a uint8_t value you can simply cast the char.
 *   Specific methods exist to read uint16_t and uint32_t values:
 *     - uint16_t USART_RxDouble(USART_TypeDef *usart);
 *     - uint32_t USART_RxDoubleExt(USART_TypeDef *usart);
 *
 * @return
 *   The character read from USARTx.
 *****************************************************************************/
char dbReadChar ()
{
	return (USART_Rx(dbpointer));
}


/**************************************************************************//**
 * @brief
 *   Read a decimal character from USARTx and convert it to a uint8_t value.
 *
 * @return
 *   The converted uint8_t value.
 *****************************************************************************/
uint8_t dbReadInt ()
{
	/* Method expects a char array ending with a null termination character */
	char value[2];
	value[0]= dbReadChar();
	value[1] = '\0';

	return (charDec_to_uint32(value));
}


/**************************************************************************//**
 * @brief
 *   Read a string (char array) from USARTx.
 *
 * @note
 *   The reading stops when a CR character is received or the maximum
 *   length (DBPRINT_BUFFER_SIZE) is reached.
 *
 * @param[in] buf
 *   The buffer to put the resulting string in.
 *   This needs to have a length of DBPRINT_BUFFER_SIZE for the function
 *   to work properly: "char buf[DBPRINT_BUFFER_SIZE];"!
 *****************************************************************************/
void dbReadLine (char *buf)
{
	for (uint32_t i = 0; i < DBPRINT_BUFFER_SIZE - 1 ; i++ )
	{
		char localBuffer = USART_Rx(dbpointer);

		/* Check if a CR character is received */
		if (localBuffer == '\r')
		{
			/* End with a null termination character, expected by the dbprintln method */
			buf[i] = '\0';
			break;
		}
		else
		{
			buf[i] = localBuffer;
		}
	}
}


/**************************************************************************//**
 * @brief
 *   Convert a uint32_t value to a hexadecimal char array (string).
 *
 * @param[out] buf
 *   The buffer to put the resulting string in.
 *   This needs to have a length of 9: "char buf[9];"!
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


/**************************************************************************//**
 * @brief
 *   Convert a uint32_t value to a decimal char array (string).
 *
 * @param[out] buf
 *   The buffer to put the resulting string in.
 *   This needs to have a length of 10: "char buf[10];"!
 *
 * @param[in] value
 *   The uint32_t value to convert to a string.
 *****************************************************************************/
void uint32_to_charDec (char *buf, uint32_t value)
{
	if (value == 0)
	{
		buf[0] = '0';
		buf[1] = '\0'; /* NULL termination character */
	}
	else
	{
		/* MAX uint32_t value = FFFFFFFFh = 4294967295d (10 decimal chars) */
		char backwardsBuf[10];

		uint32_t calcval = value;
		uint8_t length = 0;
		uint8_t lengthCounter = 0;


		/* Loop until the value is zero (separate characters 0-9) and calculate length */
		while (calcval)
		{
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


/**************************************************************************//**
 * @brief
 *   Convert a string (char array) in decimal notation to a uint32_t value.
 *
 * @note
 *   If the input is not a string (ex.: "00BA0FA1") but a char array,
 *   the input buffer (array) needs to end with NULL ('\0')!
 *
 * @param[in] buf
 *   The decimal string to convert to a uint32_t value.
 *
 * @return
 *   The resulting uint32_t value.
 *****************************************************************************/
uint32_t charDec_to_uint32 (char *buf)
{
	/* Value to eventually return */
	uint32_t value = 0;

	/* Loop until buffer is empty */
	while (*buf)
	{
		/* Get current character, increment afterwards */
		uint8_t byte = *buf++;

		/* Check if the next value we need to add can fit in a uint32_t */
		if ( (value <= 0xFFFFFFF) && ((byte - '0') <= 0xF) )
		{
			/* Convert the ASCII (decimal) character to the representing decimal value
			 * and add it to the value (which is multiplied by 10 for each position) */
			value = (value * 10) + (byte - '0');
		}
		else
		{
			/* Given buffer can't fit in uint32_t */
			return (0);
		}
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
 *   The input string can't have the prefix "0x".
 *
 * @param[in] buf
 *   The hexadecimal string to convert to a uint32_t value.
 *
 * @return
 *   The resulting uint32_t value.
 *****************************************************************************/
uint32_t charHex_to_uint32 (char *buf)
{
	/* Value to eventually return */
	uint32_t value = 0;

	/* Loop until buffer is empty */
	while (*buf)
	{
		/* Get current character, increment afterwards */
		uint8_t byte = *buf++;

		/* Convert the hex character to the 4-bit equivalent
		 * number using the ASCII table indexes */
		if (byte >= '0' && byte <= '9') byte = byte - '0';
		else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
		else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;

		/* Check if the next byte we need to add can fit in a uint32_t */
		if ( (value <= 0xFFFFFFF) && (byte <= 0xF) )
		{
			/* Shift one nibble (4 bits) to make space for a new digit
			 * and add the 4 bits (ANDing with a mask, 0xF = 0b1111) */
			value = (value << 4) | (byte & 0xF);
		}
		else
		{
			/* Given buffer can't fit in uint32_t */
			return (0);
		}
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
void USART0_RX_IRQHandler(void)
{
	/* "static" so it keeps its value between invocations */
	static uint32_t i = 0;

	/* Get and clear the pending USART interrupt flags */
	uint32_t flags = USART_IntGet(dbpointer);
	USART_IntClear(dbpointer, flags);

	/* Store incoming data into dbprint_rx_buffer */
	dbprint_rx_buffer[i++] = USART_Rx(dbpointer);

	/* Set dbprint_rxdata when a special character is received (~ full line received) */
	if ( (dbprint_rx_buffer[i - 1] == '\r') || (dbprint_rx_buffer[i - 1] == '\f') )
	{
		dbprint_rxdata = true;
		dbprint_rx_buffer[i - 1] = '\0'; /* Overwrite CR or LF character */
		i = 0;
	}

	/* Set dbprint_rxdata when the buffer is full */
	if (i >= (DBPRINT_BUFFER_SIZE - 2))
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
void USART0_TX_IRQHandler(void)
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
		if ( (i < DBPRINT_BUFFER_SIZE) && (dbprint_tx_buffer[i] != '\0') )
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
	if ( (dbprint_rx_buffer[i - 1] == '\r') || (dbprint_rx_buffer[i - 1] == '\f') )
	{
		dbprint_rxdata = true;
		dbprint_rx_buffer[i - 1] = '\0'; /* Overwrite CR or LF character */
		i = 0;
	}

	/* Set dbprint_rxdata when the buffer is full */
	if (i >= (DBPRINT_BUFFER_SIZE - 2))
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
		if ( (i < DBPRINT_BUFFER_SIZE) && (dbprint_tx_buffer[i] != '\0') )
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

