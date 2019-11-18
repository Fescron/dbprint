# dbprint

![License](https://img.shields.io/badge/license-GNU%20GPL%20v3.0-blue.svg)
![GitHub last commit](https://img.shields.io/github/last-commit/Fescron/dbprint.svg)
![GitHub Release Date](https://img.shields.io/github/release-date/Fescron/dbprint.svg)
![GitHub release](https://img.shields.io/github/release/Fescron/dbprint.svg)
![Target device](https://img.shields.io/badge/target%20device-EFM32HG322F64G-yellow.svg)
![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/Fescron/dbprint.svg)

**DeBugPrint** is a homebrew minimal low-level `println/printf` replacement. It can be used to to print text/values to `UART`without a lot of external libraries. The end goal was to use no exernal libraries (with methods like `itoa`) apart from the ones specific to the microcontroller.

**DeBugPrint** was originally designed for use on the `Silicon Labs Happy Gecko EFM32 board (EFM32HG322 -- TQFP48)` (`SLSTK3400A`) and was developed on `Simplicity Studio v4` on `Ubuntu 19.04`.

<br/>

- There are a lot of useful yet simple code-examples at https://github.com/SiliconLabs/peripheral_examples
- Click [**here**](https://fescron.github.io/dbprint/dbprint__documentation_8h.html) to find a lot of **useful information** (general info, info about keywords and datatypes, ...) about the project and C in general.
- Click [**here**](https://fescron.github.io/dbprint/dbprint_8h.html) to find **detailed information about all available methods**.

<br/>

## Table of contents

- [dbprint](#dbprint)
  - [Table of contents](#table-of-contents)
  - [1 - Installation instructions](#1---installation-instructions)
    - [1.1 - Add `dbprint` folder to your project](#11---add-dbprint-folder-to-your-project)
    - [1.2 - Add `em_usart.c` to your project (if not already added)](#12---add-emusartc-to-your-project-if-not-already-added)
    - [1.3 - Include `debug_dbprint.h` in your project's `main.c` (or another) file](#13---include-debugdbprinth-in-your-projects-mainc-or-another-file)
    - [1.4 - Start adding dbprint functionality](#14---start-adding-dbprint-functionality)
    - [1.5 - Clean & Build](#15---clean--build)
  - [2 - Enable/disable dbprint using definition in `debug_dbprint.h`](#2---enabledisable-dbprint-using-definition-in-debugdbprinth)
  - [3 - VCOM](#3---vcom)
  - [4 - Energy profiler and dbprint](#4---energy-profiler-and-dbprint)
  - [5 - Methods](#5---methods)
    - [5.1 - Definitions](#51---definitions)
    - [5.2 - Usage examples](#52---usage-examples)
      - [5.2.1 - Basic functions](#521---basic-functions)
      - [5.2.2 - More advanced functions](#522---more-advanced-functions)
      - [5.2.3 - Interrupt functionality](#523---interrupt-functionality)
  - [6 - Alternate locations of pins](#6---alternate-locations-of-pins)

<br/>

## 1 - Installation instructions

### 1.1 - Add `dbprint` folder to your project

**Copy** the `dbprint` folder containing the header and source files from this repository to your project folder.

Open the **project settings** using `File > Properties` (or right click on your project in the *Project Explorer* and choose `Properties`).

In the opened window choose `C/C++ General > Paths and Symbols` on the left.

In the tab **"Includes"**:

1. Click `Add... > Workspace...`
2. Browse to the **"dbprint"** folder in your project folder and press OK. 
3. Check the boxes *"Add to all languages"* and *"Is a workspace path"* and press OK.

<br/>

### 1.2 - Add `em_usart.c` to your project (if not already added)

In any *Simplicity Studio example project* (like **blink**) all of the header files (`.h`) for **`emlib`** are included but the source files (`.c`) are sometimes not. This needs to be done manually:

1. In the **Project Explorer** on the left, rightclick on the **`emlib`** folder under your project and select `New > File from Template`
2. Click on `Advanced>>`and check *"Link to file in the file system"*.
3. Click `Browse...`, go to `SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.4/platform/emlib/src`, select the the **`em_usart.c`** file and press OK.
4. Press FINISH.

<br/>

### 1.3 - Include `debug_dbprint.h` in your project's `main.c` (or another) file

```C
#include "debug_dbprint.h"
```

<br/>

### 1.4 - Start adding dbprint functionality

It's advised to **surround dbprint statements in your code with `IF ... ENDIF`** so they can be enabled/disabled by setting the definition `DEBUG_DBPRINT` in `debug_dbprint.h` to `1` or `0`:

```C
#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */

dbprint_INIT(USART1, 4, true, false); /* Initialize dbprint for use with VCOM */

dbprintln("Hello world!"); /* An example of a dbprint statement */

#endif /* DEBUG_DBPRINT */
```

More information about this can be found in ["2 - Enable/disable dbprint using definition in `debug_dbprint.h`"](#2---enabledisable-dbprint-using-definition-in-debugdbprinth).

<br/>

### 1.5 - Clean & Build

Perform a *clean and build* action to fix errors that would occur when the project would just get *build* after the dbprint files are added.

1. Click `Project > Clean...`
2. Select *"Clean projects selected below"* and check the current project.
3. Check *"Start a build immediately"* while *"Build only the selected projects"* is selected.
4. Press OK.

<br/>

## 2 - Enable/disable dbprint using definition in `debug_dbprint.h`

In the file `debug_dbprint.h` dbprint UART functionality can be enabled/disabled with the definition `#define DEBUG_DBPRINT`. If it's value is `0`, all dbprint functionality is disabled. This means that the **only header file to include in your projects** for dbprint to work is `#include debug_dbprint.h`

Again, it's advised to **surround dbprint statements in your code with `IF ... ENDIF`** so they can be enabled/disabled by setting the definition `DEBUG_DBPRINT` in `debug_dbprint.h` to `1` or `0`:

```C
#if DEBUG_DBPRINT == 1 /* DEBUG_DBPRINT */

dbprintln("Hello world!"); /* An example of a dbprint statement */

#endif /* DEBUG_DBPRINT */
```

<br/>

## 3 - VCOM

 When using `dbprint` functionality, the following settings are used and can't be changed without editing the source code:

- `Baudrate = 115200`
- `8 databits`
- `1 stopbit`
- `No parity`

<br/>

VCOM is an on-board (on the `SLSTK3400A`) **UART to USB converter** alongside the Segger J-Link debugger. It's connected with microcontroller pins `PA0` (RX) and `PF2` (TX). This converter can then be used with [Putty](https://www.putty.org/) or another serial port program.

When you want to **debug using VCOM with interrupt functionality disabled**, you can use the following initialization settings:

```C
dbprint_INIT(USART1, 4, true, false);
```

Setting the third argument to `true` indicates to the code that `PA9`(`EFM_BC_EN`) should be set high to enable the isolation switch on the PCB of the Happy Gecko to link `PA0` (RX) and `PF2` (TX) to the debugger. **Don't use this pin yourself if you want to make use of the on-board UART to USB converter!**

<br/>

## 4 - Energy profiler and dbprint

The Energy profiler in Simplicity Studio seems to use VCOM somehow. Use an **external UART adapter** if both the energy profiler and UART debugging are necessary at the same time!

Don't forget to change the `INIT` arguments to select the correct pins if necessary! (see section ["6 - Alternate locations of pins"](#6---alternate-locations-of-pins)).

If the energy profiler was used and the code functionality was switched, physically re-plug the board to make sure VCOM UART starts working again!

<br/>

<!--
The following instructions are used when you want to be able to use `dbprint` in every project you create. Place the source files somewhere safe so you don't accidentally delete them!

Open the project settings using `File > Properties > C/C++ General > Paths and Symbols`.

In the tab **"Includes"**:

1. Click `Add... > File system...`
2. Browse to the **"dbprint-inc"** folder and press OK. 
3. Check *"Add to all languages"* and press OK.

In the tab **"Source Location"**:

1. Click `Link Folder...`
2. Check *"Link to folder in the file system"*
3. Click `Browse...`, select the the **"dbprint-scr"** folder and press OK.
-->

## 5 - Methods

### 5.1 - Definitions

This is a list of all available methods. **Detailed documentation about them can be found [here](https://fescron.github.io/dbprint/dbprint_8h.html).**

```C
void dbprint_INIT(USART_TypeDef* pointer, uint8_t location, bool vcom, bool interrupts);
 
void dbAlert(void);
void dbClear(void);

void dbprint(char *message);
void dbprintln(char *message);

void dbprintInt(int32_t value);
void dbprintlnInt(int32_t value);

void dbprintInt_hex(int32_t value);
void dbprintlnInt_hex(int32_t value);

void dbprint_color(char *message, dbprint_color_t color);
void dbprintln_color(char *message, dbprint_color_t color);

void dbinfo(char *message);
void dbwarn(char *message);
void dbcrit(char *message);

void dbinfoInt(char *message1, int32_t value, char *message2);
void dbwarnInt(char *message1, int32_t value, char *message2);
void dbcritInt(char *message1, int32_t value, char *message2);

void dbinfoInt_hex(char *message1, int32_t value, char *message2);
void dbwarnInt_hex(char *message1, int32_t value, char *message2);
void dbcritInt_hex(char *message1, int32_t value, char *message2);

char dbReadChar(void);
uint8_t dbReadInt(void);
void dbReadLine(char *buf);

bool dbGet_RXstatus(void);
void dbGet_RXbuffer(char *buf);
```

<br/>

### 5.2 - Usage examples

#### 5.2.1 - Basic functions

```C
dbprint_INIT(USART1, 4, true, false); /* Initialize UART1 on VCOM, no interrupts*/
```

```C
dbprint("Hello World");    /* Print text to uart */
dbprintln("");             /* Go to next line */
dbprintln("Hello World");  /* Print text to uart and go to the next line */

dbinfo("Info.");           /* Print an info message (prefix "INFO: ") */
dbwarn("Warning.");        /* Print a warning message in yellow (prefix "WARN: ") */
dbcrit("Critical error."); /* Print a critical error message in red (prefix "CRIT: ") */
```

```C
uint32_t value = 42;

/* Print "unsigned int" value in decimal notation */
dbprintInt(value);       /* Stay on the current line */
dbprintlnInt(value);     /* Go to next line */

/* Print "unsigned int" value in hexadecimal notation */
dbprintInt_hex(value);   /* Stay on the current line */
dbprintlnInt_hex(value); /* Go to next line */

/* The methods above also work for printing "signed int" values like: */
int32_t intValue = -42;
```

```C
/* Read a character and check wich one it is */
dbprint("Type 'y' or 'n': ");
char test = dbReadChar();
if (test == 'y') dbprintln("Yes");
if (test == 'n') dbprintln("No");

/* Read a character and convert it to a uint8_t value, print it afterwards */
dbprint("Type a number (1 character): ");
uint8_t test2 = dbReadInt();
dbprintlnInt(test2);

/* Read a line and print it, press enter to stop typing.
 * It also stops when DBPRINT_BUFFER_SIZE is reached. */
dbprint("Type a line: ");
char testArray[DBPRINT_BUFFER_SIZE];
dbReadLine(testArray);
dbprintln(testArray);
```

<br/>

#### 5.2.2 - More advanced functions

```C
dbAlert(); /* Let the console make an "alert" (bell) sound */
dbClear(); /* Clear the console window */
```

```C
dbprint_color("Hello World", RED);   /* Print red text to uart */
dbprintln("");                       /* Go to next line */
dbprintln_color("Hello World", RED); /* Print red text to uart and go to the next line */
```

```C
uint32_t value = 42;

/* Print an info message with prefix "INFO: " where a value
   in decimal notation is enclosed between two strings */
dbinfoInt("Info = ", value, " [unit of value]");

/* Print an info message with prefix "INFO: " where a value
   in hexadecimal notation is enclosed between two strings */
dbinfoInt_hex("Info = ", value, " [unit of value]");

/* Print a warning message in yellow with prefix "WARN: " where a value
   in decimal notation is enclosed between two strings */
dbwarnInt("Warning = ", value, " [unit of value]");

/* Print a warning message in yellow with prefix "WARN: " where a value
   in hexadecimal notation is enclosed between two strings */
dbwarnInt_hex("Warning = ", value, " [unit of value]");

/* Print a critical error message in red with prefix "CRIT: " where a value
  in decimal notation is enclosed between two strings */
dbcritInt("Critical error = ", value, " [unit of value]");

/* Print a critical error message in red with prefix "CRIT: " where a value
  in hexadecimal notation is enclosed between two strings */
dbcritInt_hex("Critical error = ", value, " [unit of value]");
```

<br/>

#### 5.2.3 - Interrupt functionality

Dbprint can also be put in **interrupt mode** by using the `INIT` arguments below, using VCOM for this example. Then, **received characters will be automatically stored in an internal buffer** for later use.

```C
dbprint_INIT(USART1, 4, true, true); /* Initialize dbprint on VCOM, interrupt mode */
```

A *getter* (`dbGet_RXstatus();`) can be used to check if there is received data in this internal buffer and another *getter* (`dbGet_RXbuffer();`) can be used to copy the data from this internal buffer to another one.

An example using these two getters is depicted below and can be put in, for example, the `main.c` file.

```C
bool received = dbGet_RXstatus(); /* Check if there is data received in the buffer */

/* If we received data do the following */
if (received)
{
  char buf[DBPRINT_BUFFER_SIZE]; /* Create a temporary buffer to store the received data in */
  dbGet_RXbuffer(buf); /* Copy the received data to this temporary buffer */
  dbprintln(buf); /* Print the contents of this temporary buffer in the terminal */
}
```

<br/>

## 6 - Alternate locations of pins

In C, pin selection/routing happens at the end of initialization methods using statements like:

```C
USART1->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC0;
```

If you use dbprint you don't really need to worry about this but you need to make sure you **select the correct location when calling the `dbprint_INIT` method**.

The location numbers and corresponding `RX`and `TX`pins for `USART0`and `USART1` (EFM32HG322) are given below.

| Location |  #0  |  #1  |  #2  |  #3  |  #4  |  #5  |  #6  |
| -------- |:----:|:----:|:----:|:----:|:----:|:----:|:----:| 
| US0_RX   | PE11 |      | PC10 | PE12 | PB8  | PC1  | PC1  |
| US0_TX   | PE10 |      |      | PE13 | PB7  | PC0  | PC0  |
| US1_RX   | PC1  |      | PD6  | PD6  | PA0  | PC2  |      |
| US1_TX   | PC0  |      | PD7  | PD7  | PF2  | PC1  |      |

<br/>

VCOM:
 - `USART1 #4` (`USART0` can't be used)
 - RX - `PA0`
 - TX - `PF2`
 - Isolation switch - `PA9` (`EFM_BC_EN`) <br/> **Don't use this pin yourself when using the on-board UART to USB converter!**
