/***************************************************************************//**
 * @file debug_dbprint.h
 * @brief Enable or disable printing to UART.
 * @details
 *   This header file should be called in every other file where there are UART
 *   debugging statements. Depending on the value of `DEBUG_DBPRINT`,
 *   UART statements are enabled or disabled.
 * @version 1.0
 * @author Brecht Van Eeckhoudt
 ******************************************************************************/


/* Include guards prevent multiple inclusions of the same header */
#ifndef _DEBUG_DBPRINT_H_
#define _DEBUG_DBPRINT_H_


/** Public definition to enable/disable UART debugging
 *    @li `1` - Enable the UART debugging statements.
 *    @li `0` - Remove all UART debugging statements from the uploaded code. */
#define DEBUG_DBPRINT 1


#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */
#include "dbprint.h"
#endif /* DEBUG_DBPRINT */


#endif /* _DEBUG_DBPRINT_H_ */
