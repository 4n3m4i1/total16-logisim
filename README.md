## NOTE: This is unfinished and may remain in the current broken state
## Bigger and better things are coming :)
# total16-logisim
16-bit CPU design made in Logisim 2.7.1

16 bit instruction set outlined in total16InstructionSet.txt

Current Abilities:
  - Basic arithmetic
  - Loading 5 and 10 bit immediates
  
Soon to Come:
  - Load and store word from RAM
  - Load from program memory
  - Constants and Data Types
  - A more complete assembler
  
Assembler:
  - Reads in any file (hopefully in appropriate format..) and outputs a binary file with no extention,
  though one can be specified in the command line. 
  - A hex file is also output mirroring the binary file but with a .hex extention.
  - Checks are done to make sure instruction format and opcodes are correct, semi detailed error messages will be produced
  - CMD line arguments: -d, -D, -A, -h as the last argument will provide detailed outputs of each file/line processed
  - Completely random formats are not checked for, make sure to provide only expected inputs.
  
  Instruction Basics:
  - 2 types:
  - 6 bit opcode, 5 bit arg0, 5 bit arg1/immediate
  - 6 bit opcode, 10 bit immediate
  
  # Running a program
  - Reference example program for a guide
  - Write a basic asm file (note: comparison, jump, and lw/sw hardware is unfinished if this message is here)
  - run the assembler with the command line arguments: <source file path> <destination file path> <optional arguments>
  - copy the contents of the hex file into the ROM, easily done by clicking ROM, saving the file, then copying contents over
  - Make sure to have a NOP at the beginning of your program!
  
