/***************************************************************************//**
 * @file dbprint.h
 * @brief Homebrew println/printf replacement "DeBugPRINT".
 * @version 2.3
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _DBPRINT_H_
#define _DBPRINT_H_


#include <stdint.h>     /* (u)intXX_t */
#include <stdbool.h>    /* "bool", "true", "false" */
#include "em_cmu.h"     /* Clock Management Unit */
#include "em_gpio.h"    /* General Purpose IO (GPIO) peripheral API */
#include "em_usart.h"   /* Universal synchr./asynchr. receiver/transmitter (USART/UART) Peripheral API */


/* Buffer size */
#define DBPRINT_BUFFER_SIZE 80


/* Global variables */
extern USART_TypeDef* dbpointer;

extern volatile bool dbprint_rxdata; /* true if there is data received */
extern volatile char dbprint_rx_buffer[DBPRINT_BUFFER_SIZE];
extern volatile char dbprint_tx_buffer[DBPRINT_BUFFER_SIZE];


/* Prototypes */
void dbprint_INIT (USART_TypeDef* pointer, uint8_t location, bool vcom, bool interrupts);
void dbAlert ();
void dbClear ();

void dbprint (char *message);
void dbprintln (char *message);

void dbprintUint (uint32_t value);
void dbprintUint_hex (uint32_t value);
void dbprintInt (uint8_t radix, int32_t value);

void uint32_to_charHex (char *buf, uint32_t value, bool spacing);
void uint32_to_charDec (char *buf, uint32_t value);
uint32_t charDec_to_uint32 (char *buf);
uint32_t charHex_to_uint32 (char *buf);


#endif /* _DBPRINT_H_ */
