# Flashlib
A flash programming and EEPROM emulation utility library for PIC32MX devices.
So far this library only supports the PIC32MX1xx/2xx/5xx 64/100pin devices from datasheet [D60001290](https://ww1.microchip.com/downloads/en/DeviceDoc/PIC32MX1XX2XX5XX%2064100-PIN_Family_60001290F.pdf). 

This library assumes you have the `xc32-gcc` compiler chain installed. If you haven't, you can find it [here](https://www.microchip.com/en-us/development-tools-tools-and-software/mplab-xc-compilers).

## Supported Functions
 - `eeprom_read_word` Reads a word from emulated eeprom, if enabled
 - `eeprom_write_word` Writes a word to emulated eeprom, if enabled
 - `flash_read_word` Reads a word from program memory flash
 - `flash_write_word` Writes a word to program memory flash
 - `flash_write_doubleword` Writes a doubleword to program memory flash, if enabled
 - `flash_write_row` Writes a row of data to program memory flash
 - `flash_erase_page` Erases a page of program memory flash
 - `flash_erase_all_program_memory` Erases all of program memory flash - Including emulated eeprom sector

## Use in your CMake projects
Simply add the folder as a subdirectory. Don't forget to set the compiler to the `xc32-gcc` compiler.
```
add_subdirectory(path/to/flashlib)
...
target_link_libraries(target flashlib)
```
You can now include the `flashlib.h` file and use the functions defined therein.

## EEPROM Emulation
You can specify at compile-time what section of flash you want to dedicate to eeprom emulation.
The following cmake variables are available: 
```
-DENABLE_EEPROM_EMU              # Enabling compilation of eeprom emulation
-DEEPROM_SECTOR_START=<physaddr> # Determines start address for eeprom dedicated flash memory
-DEEPROM_SECTOR_END=<physaddr>   # Determines end address for eeprom dedicated flash memory
```
The start and end sector variables are required if you want to use eeprom emulation. If `ENABLE_EEPROM_EMU` is not defined, no need to define the start and end sectors. Note that all flash addresses should be in kernel-space (KSEG0/1).
Below is an example usage:
```
cmake -DENABLE_EEPROM_EMU=1 -DEEPROM_SECTOR_START=0x9D070000 -DEEPROM_SECTOR_END=0x9D07FFFF ..
```

## Flash programming support
If your MCU supports double word programming (not all do) you can compile with support for that with the `ENABLE_DOUBLEWORD_PROGRAMMING` cmake flag variable. 

# Contribute
If you have any additions you want submitted, feel free to open up a pull request or issue. Make sure to add yourself in the list of authors in the appropriate file(s). 

### Notes
CMake tries to check if the provided compiler i "valid", but `xc32-gcc` does not support the standard `-rdynamic` flag.
This check can be sidestepped by providing cmake with the `-DCMAKE_C_COMPILER_WORKS=1` flag.
