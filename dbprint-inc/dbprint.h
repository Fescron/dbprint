/***************************************************************************//**
 * @file dbprint.h
 * @brief Homebrew println/printf replacement "DeBugPRINT"
 * @version 2.1
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/

#include <stdint.h>  	/* (u)intXX_t */
#include <stdbool.h> 	/* "bool", "true", "false" */
#include "em_cmu.h"		/* Clock Management Unit */
#include "em_gpio.h"    /* General Purpose IO (GPIO) peripheral API */
#include "em_usart.h"	/* Universal synchr./asynchr. receiver/transmitter (USART/UART) Peripheral API */

/* Buffer size */
#define DBPRINT_BUFFER_SIZE 80


/* Global variables */
extern USART_TypeDef* dbpointer;

/* Volatile global variables
 *   The “volatile” type indicates to the compiler that the data is not normal memory,
 *   and could actually change at unexpected times. Hardware registers are often volatile,
 *   and so are variables which get changed in interrupts.
 *
 * Extern
 *   Declare the global variables in headers and actually define them in the appropriate source file.
 */
extern volatile bool dbprint_rxdata; /* true if there is data received */
extern volatile char dbprint_rx_buffer[DBPRINT_BUFFER_SIZE];
extern volatile char dbprint_tx_buffer[DBPRINT_BUFFER_SIZE];


/* Prototypes */
void dbprint_INIT (USART_TypeDef* pointer, uint8_t location, bool vcom, bool interrupts);
void dbAlert ();
void dbClear ();
void dbprint (char *message);
void dbprintUint (uint8_t radix, uint32_t value);
void dbprintInt (uint8_t radix, int32_t value);
void dbprintln (char *message);
void uint32_to_charHex (char *buf, uint32_t value, bool spacing);
void uint32_to_charDec (char *buf, uint32_t value);
