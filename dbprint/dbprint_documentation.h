/***************************************************************************//**
 * @file dbprint_documentation.h
 * @brief This file contains useful documentation closely related to dbprint.
 * @version 7.0
 * @author Brecht Van Eeckhoudt
 *
 * ******************************************************************************
 *
 * @section DeBugPrint
 *
 *   DeBugPrint is a homebrew minimal low-level println/printf replacement.
 *   It can be used to to print text/values to uart without a lot of external
 *   libraries. The end goal was to use no external libraries (with methods
 *   like `itoa`) apart from the ones specific to the microcontroller.
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
 * @section ENABLE Enable/disable dbprint using definition in `debug_dbprint.h`
 *
 *   In the file `debug_dbprint.h` **dbprint UART functionality can be enabled/disabled**
 *   with the definition `#define DEBUG_DBPRINT`. If it's value is `0`, all dbprint
 *   functionality is disabled.
 *
 *   @warning
 *     This means that the **only header file to include in your projects** for
 *     dbprint to work is@n
 *     `#include debug_dbprint.h`
 *
 *   @note
 *     If you also want to use this definition to enable/disable dbprint statements
 *     in your code, please use the following convention:@n
 *     `#if DEBUG_DBPRINT == 1 // DEBUG_DBPRINT `@n
 *     `<your source code dbprint statements go here>`@n
 *     `#endif // DEBUG_DBPRINT`
 *
 * ******************************************************************************
 *
 * @section DBPRINT More info about dbprint (and VCOM)
 *
 *   @note
 *     When using `dbprint` functionality, the following settings are used and
 *     can't be changed without editing the source code:@n
 *       - Baudrate = 115200
 *       - 8 databits
 *       - 1 stopbit
 *       - No parity
 *
 *   VCOM is an on-board (SLSTK3400A) UART to USB converter alongside the Segger
 *   J-Link debugger, connected with microcontroller pins `PA0` (RX) and `PF2` (TX).
 *   This converter can then be used with Putty or another serial port program.
 *
 *   @note
 *     When you want to debug using VCOM with interrupt functionality disabled, you
 *     can use the following initialization settings:@n
 *     `dbprint_INIT(USART1, 4, true, false);`
 *
 *   @warning
 *     Setting the third argument to `true` indicates to the code that `PA9`
 *     (`EFM_BC_EN`) should be set high to enable the isolation switch on the PCB
 *     of the Happy Gecko to link `PA0` (RX) and `PF2` (TX) to the debugger. Don't
 *     use this pin yourself if you want to make use of the on-board UART to USB converter!
 *
 * ******************************************************************************
 *
 * @section ENERGY Energy profiler and dbprint
 *
 *   The Energy profiler in Simplicity Studio seems to use VCOM  somehow, change
 *   to using an external UART adapter if both the energy profiler and UART
 *   debugging are necessary at the same time!
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
 *     - Isolation switch - `PA9` (`EFM_BC_EN`)
 *       (don't use this pin yourself when using the on-board UART to USB converter)
 *
 * ******************************************************************************
 *
 * @section Keywords
 *
 *   @subsection Volatile
 *
 *   The `volatile` type indicates to the compiler that the data is not normal memory,
 *   and could change at unexpected times. Hardware registers are often volatile,
 *   and so are variables which get changed in interrupts.@n
 *   Volatile variables are stored in *RAM*.
 *
 *   @subsection Static
 *
 *   @subsubsection VARIABLE Static variable
 *
 *   During compile time (this is why we can't use variable length array's) memory
 *   gets reserved for this variable. The data itself gets put in the *data* segment
 *   of the memory (regular variables are put in the *stack* segment).@n
 *   It's best to keep the use of `static` variables to a minimum. One should ask
 *   himself the question if it's necessary to keep the variable constantly in the
 *   memory. If the answer is yes, a `static` variable is acceptable.@n
 *   A **static variable inside a function** keeps its value between invocations.
 *
 *   @subsubsection FUNCTION Static global function
 *
 *   The function is only "seen" in the file it's declared in. This means `static`
 *   can be used for methods the same way `private` is used for certain methods in C++.
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
 *   - `-128` = `0x80` = `0b1000 0000` (If the left most bit is one, the sign of the number is negative)
 *   -  `127` = `0x7F` = `0b0111 1111`
 *
 ******************************************************************************/
