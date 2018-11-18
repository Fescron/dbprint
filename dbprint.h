/***************************************************************************//**
 * @file dbprint.h
 * @brief Selfmade println replacement
 * @version 1.2
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/

#include <stdint.h>  	/* (u)intXX_t */
#include <stdbool.h> 	/* "bool", "true", "false" */
#include "em_cmu.h"		/* Clock Management Unit */
#include "em_usart.h"	/* Universal synchr./asynchr. receiver/transmitter (USART/UART) Peripheral API */


/* Prototypes */

/**************************************************************************//**
 * @brief Initialize USARTx (serial output debugging: Baudrate = 115200 -- eight databits -- one stopbit -- no parity)
 * @note Location example: US1_TX @ datasheet: #4 = USART_ROUTE_LOCATION_LOC4
 *
 * @param pointer USARTx pointer
 * @param location Location for the pin routing
 * @param vcom If true: route TX and RX to "Virtual com port (CDC)" on Happy Ghecko board (PA9 is also set high to enable the isolation switch)
 *****************************************************************************/
void dbprint_INIT (USART_TypeDef* pointer, uint8_t location, bool vcom);


/**************************************************************************//**
 * @brief Print the BELL character to USARTx to sound an ALERT
 *****************************************************************************/
void dbAlert();


/**************************************************************************//**
 * @brief Print a string (char array) to USARTx
 *
 * @param message The message to display
 *****************************************************************************/
void dbprint (char *message);


/**************************************************************************//**
 * @brief Print a uint32_t to USARTx
 *
 * @param radix 10 for decimal, 16 for hexadecimal
 * @param value The value to display
 *****************************************************************************/
void dbprintUint (uint8_t radix, uint32_t value);


/**************************************************************************//**
 * @brief Print a int32_t to USARTx
 *
 * @param radix 10 for decimal, 16 for hexadecimal
 * @param value The value to display
 *****************************************************************************/
void dbprintInt (uint8_t radix, int32_t value);


/**************************************************************************//**
 * @brief Print a string (char array) to USARTx and go to the next line
 *
 * @param message The message to display
 *****************************************************************************/
void dbprintln (char *message);


/**************************************************************************//**
 * @brief Convert uint32_t to HEX char notation (string)
 *
 * @param buf The buffer (needs to be: "char buf[9];")
 * @param value The uint32_t value
 * @param spacing True if there needs to be added spacing between the eight HEX chars
 *****************************************************************************/
void uint32_to_charHex (char *buf, uint32_t value, bool spacing);


/**************************************************************************//**
 * @brief Convert uint32_t to DEC char notation (string)
 *
 * @param buf The buffer (needs to be: "char buf[10];")
 * @param value The uint32_t value
 *****************************************************************************/
void uint32_to_charDec (char *buf, uint32_t value);
