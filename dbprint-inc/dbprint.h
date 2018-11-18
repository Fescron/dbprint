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
void dbprint_INIT (USART_TypeDef* pointer, uint8_t location, bool vcom);
void dbAlert();
void dbprint (char *message);
void dbprintUint (uint8_t radix, uint32_t value);
void dbprintInt (uint8_t radix, int32_t value);
void dbprintln (char *message);
void uint32_to_charHex (char *buf, uint32_t value, bool spacing);
void uint32_to_charDec (char *buf, uint32_t value);
