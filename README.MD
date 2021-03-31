# Flashlib
A flash programming and EEPROM emulation utility library for PIC32MX devices.
So far this library only supports the PIC32MX1xx/2xx/5xx 64/100pin devices from datasheet [D60001290](https://ww1.microchip.com/downloads/en/DeviceDoc/PIC32MX1XX2XX5XX%2064100-PIN_Family_60001290F.pdf). 

This library assumes you have the `xc32-gcc` compiler chain installed. If you haven't, you can find it [here](https://www.microchip.com/en-us/development-tools-tools-and-software/mplab-xc-compilers).

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
The start and end sector variables are required if you want to use eeprom emulation. If `ENABLE_EEPROM_EMU` is not defined, no need to define the start and end sectors.
Below is an example usage:
```
cmake -DENABLE_EEPROM_EMU=1 -DEEPROM_SECTOR_START=0x1D070000 -DEEPROM_SECTOR_END=0x1D07FFFF ..
```

### Notes
CMake tries to check if the provided compiler i "valid", but `xc32-gcc` does not support the standard `-rdynamic` flag.
This check can be sidestepped by providing cmake with the `-DCMAKE_C_COMPILER_WORKS=1` flag.
