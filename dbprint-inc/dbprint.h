/***************************************************************************//**
 * @file dbprint.h
 * @brief Homebrew println/printf replacement "DeBugPRINT".
 * @version 3.8
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


/* ANSI colors */
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RESET   "\x1b[0m"


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

void dbprintInt (int32_t value);
void dbprintlnInt (int32_t value);

void dbprintInt_hex (int32_t value);
void dbprintlnInt_hex (int32_t value);

void dbprint_color (char *message, uint8_t color);
void dbprintln_color (char *message, uint8_t color);

void dbinfo (char *message);
void dbwarn (char *message);
void dbcrit (char *message);

void dbinfoInt (char *message1, int32_t value, char *message2);
void dbwarnInt (char *message1, int32_t value, char *message2);
void dbcritInt (char *message1, int32_t value, char *message2);

void dbinfoInt_hex (char *message1, int32_t value, char *message2);
void dbwarnInt_hex (char *message1, int32_t value, char *message2);
void dbcritInt_hex (char *message1, int32_t value, char *message2);

char dbReadChar ();
uint8_t dbReadInt ();
void dbReadLine (char *buf);

void uint32_to_charHex (char *buf, uint32_t value, bool spacing);
void uint32_to_charDec (char *buf, uint32_t value);

uint32_t charDec_to_uint32 (char *buf);
uint32_t charHex_to_uint32 (char *buf);


#endif /* _DBPRINT_H_ */
