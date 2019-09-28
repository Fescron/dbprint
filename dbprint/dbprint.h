/***************************************************************************//**
 * @file dbprint.h
 * @brief Homebrew println/printf replacement "DeBugPRINT".
 * @version 5.0
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _DBPRINT_H_
#define _DBPRINT_H_


/* Includes necessary for this header file */
#include <stdint.h>  /* (u)intXX_t */
#include <stdbool.h> /* "bool", "true", "false" */


/** Public definition to configure the buffer size. */
#define DBPRINT_BUFFER_SIZE 80


/** Enum type for the color selection. */
typedef enum dbprint_colors
{
	RED,
	GREEN,
	BLUE,
	CYAN,
	MAGENTA,
	YELLOW,
	DEFAULT_COLOR
} dbprint_color_t;


/** Public variable to store the settings (pointer). */
extern USART_TypeDef* dbpointer;


/* Public prototypes */
void dbprint_INIT (USART_TypeDef* pointer, uint8_t location, bool vcom, bool interrupts);

void dbAlert (void);
void dbClear (void);

void dbprint (char *message);
void dbprintln (char *message);

void dbprintInt (int32_t value);
void dbprintlnInt (int32_t value);

void dbprintInt_hex (int32_t value);
void dbprintlnInt_hex (int32_t value);

void dbprint_color (char *message, dbprint_color_t color);
void dbprintln_color (char *message, dbprint_color_t color);

void dbinfo (char *message);
void dbwarn (char *message);
void dbcrit (char *message);

void dbinfoInt (char *message1, int32_t value, char *message2);
void dbwarnInt (char *message1, int32_t value, char *message2);
void dbcritInt (char *message1, int32_t value, char *message2);

void dbinfoInt_hex (char *message1, int32_t value, char *message2);
void dbwarnInt_hex (char *message1, int32_t value, char *message2);
void dbcritInt_hex (char *message1, int32_t value, char *message2);

char dbReadChar (void);
uint8_t dbReadInt (void);
void dbReadLine (char *buf);

bool dbGetRX_status (void);
void dbSetAndSend_TXbuffer (char *message);
void dbGetAndClear_RXbuffer (char *buf);


#endif /* _DBPRINT_H_ */
