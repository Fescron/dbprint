# dbprint

**DeBugPrint** is a homebrew minimal low-level `println/printf` replacement. It can be used to to print text/values to `uart`without a lot of external libraries. The end goal is to use no exernal libraries apart from the ones specific to the microcontroller.

**DeBugPrint** is originally designed for use on the `Silicion Labs Happy Gecko EFM32 board (EFM32HG322 -- TQFP48)`.


## Installation instructions (Simplicity Studo v4)

`File > Properties > C/C++ General > Paths and Symbols`

In the tab **"Includes"**:
1. Click `Add... > File system...`
2. Browse to the **"dbprint-inc"** folder and press OK. 
3. Tick *"Add to all languages"* and press OK.

In the tab **"Source Location"**:
1. Click `Link Folder...`
2. Tick *"Link to folder in the file system"*
3. Click `Browse...`, select the the **"dbprint-scr"** folder and press OK.
