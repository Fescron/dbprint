/***************************************************************************//**
 * @file dbprint_documentation.h
 * @brief This file contains useful documentation.
 * @version 1.0
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @mainpage
 *
 * ******************************************************************************
 *
 * @section Doxygen
 *
 *   These files have been made with [**Doxygen**](www.doxygen.org) comments above
 *   all methods to generate documentation. A combination of **Doxygen-specific tags**
 *   (preceded with an `@` symbol) and **markdown syntax** are supported to generate,
 *   for example, HTML-files.
 *
 * ******************************************************************************
 *
 * @section SETTINGS Settings using definition in `debug_dbprint.h`
 *
 *   In the file `debug_dbprint.h` **dbprint UART functionality can be enabled/disabled**
 *   with the definition `#define DEBUG_DBPRINT`. If it's value is `0`, all dbprint
 *   statements are disabled throughout the source code because they're all surrounded
 *   with `#if DEBUG_DBPRINT == 1 ... #endif` checks.
 *
 * ******************************************************************************
 *
 * @section DEBUG Debugging using VCOM
 *
 *   The Energy profiler in Simplicity Studio seems to use VCOM (on-board UART
 *   to USB converter) somehow, change to using an external UART adapter if both
 *   the energy profiler and UART debugging are necessary at the same time!
 *
 *   If the energy profiler was used and the code functionality was switched,
 *   physically re-plug the board to make sure VCOM UART starts working again!
 *
 * ******************************************************************************
 *
 * @section UART Alternate UART Functionality Pinout
 *
 *   |  Location  |  `#0`  |  `#1`  |  `#2`  |  `#3`  |  `#4`  |  `#5`  |  `#6`  |
 *   | ---------- | ------ | ------ | ------ | ------ | ------ | ------ | ------ |
 *   |  `US0_RX`  | `PE11` |        | `PC10` | `PE12` | `PB08` | `PC01` | `PC01` |
 *   |  `US0_TX`  | `PE10` |        |        | `PE13` | `PB07` | `PC00` | `PC00` |
 *   |            |        |        |        |        |        |        |        |
 *   |  `US1_RX`  | `PC01` |        | `PD06` | `PD06` | `PA00` | `PC02` |        |
 *   |  `US1_TX`  | `PC00` |        | `PD07` | `PD07` | `PF02` | `PC01` |        |
 *
 *   VCOM:
 *     - USART1 #4 (USART0 can't be used)
 *     - RX - `PA0`
 *     - TX - `PF2`
 *
 * ******************************************************************************
 *
 * @section DBPRINT More info about `dbprint`
 *
 *   When using `dbprint` functionality, the following settings are used:
 *     - Baudrate = 115200
 *     - 8 databits
 *     - 1 stopbit
 *     - No parity
 *
 *   When you want to debug using VCOM with interrupt functionality disabled, you
 *   can use the following initialization settings: `dbprint_INIT(USART1, 4, true, false);`
 *
 * ******************************************************************************
 *
 * @section Keywords
 *
 *   @subsection Volatile
 *
 *   The `volatile` type indicates to the compiler that the data is not normal memory,
 *   and could change at unexpected times. Hardware registers are often volatile,
 *   and so are variables which get changed in interrupts.
 *
 *   @subsection Extern
 *
 *   Declare the global variables in headers (and use the `extern` keyword there)
 *   and actually define them in the appropriate source file.
 *
 *   @subsection Static
 *
 *   - **Static variable inside a function:** The variable keeps its value between invocations.
 *   - **Static global variable or function:** The variable or function is only "seen" in the file it's declared in.
 *
 * ******************************************************************************
 *
 * @section DATA Bits, bytes, nibbles and unsigned/signed integer value ranges
 *
 *   - 1 nibble = 4 bits (`0b1111`      = `0xF` )
 *   - 1 byte   = 8 bits (`0b1111 1111` = `0xFF`)
 *
 *  | Type       | Alias            | Size    | Minimum value  | Maximum value                 |
 *  | ---------- | ---------------- | ------- | -------------- | ----------------------------- |
 *  | `uint8_t`  | `unsigned char`  | 1 byte  | 0              | 255 (`0xFF`)                  |
 *  | `uint16_t` | `unsigned short` | 2 bytes | 0              | 65 535 (`0xFFFF`)             |
 *  | `uint32_t` | `unsigned int`   | 4 bytes | 0              | 4 294 967 295 (`0xFFFF FFFF`) |
 *  |            |                  |         |                |                               |
 *  | `int8_t`   | `signed char`    | 1 byte  | -128           | 127                           |
 *  | `int16_t`  | `signed short`   | 2 bytes | -32 768        | 32 767                        |
 *  | `int32_t`  | `signed int`     | 4 bytes | -2 147 483 648 | 2 147 483 647                 |
 *
 ******************************************************************************/
