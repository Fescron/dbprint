# dbprint

**DeBugPrint** is a selfmade `println/printf` replacement. It's a minimal implementation to print text/values to `uart`without a lot of external libraries (the goal is to do all of the conversions with own libraries). It's originally designed for use on the **Silicion Labs Happy Gecko EFM32 board** (`EFM32HG322 -- TQFP48`).

### Installation instructions (Simplicity Studo v4)

`File > Properties > C/C++ General > Paths and Symbols`

In the tab **"Includes"**:
1. Click `Add... > File system...`
2. Browse to the **"dbprint-inc"** folder and press OK. 
3. Tick `Add to all languages` and press OK.

In the tab **"Source Location"**:
1. Click `Link Folder...`
2. Tick `Link to folder in the file system`
3. Click `Browse...`, select the the **"dbprint-scr"** folder and press OK.
