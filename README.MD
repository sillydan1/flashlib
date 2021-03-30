# Flashlib
A flash programming and EEPROM emulation utility library for PIC32MX devices

This library assumes you have the `xc32-gcc` compiler chain installed. If you haven't, you can find it [here](https://www.microchip.com/en-us/development-tools-tools-and-software/mplab-xc-compilers).

## Use in your CMake projects
Simply add the folder as a subdirectory. Don't forget to set the compiler to the `xc32-gcc` compiler.
```
add_subdirectory(path/to/flashlib)
...
target_link_libraries(target flashlib)
```
You can now include the `flashlib.h` file and use the functions defined therein.

### Notes
CMake tries to check if the provided compiler i "valid", but `xc32-gcc` does not support the standard `-rdynamic` flag.
This check can be sidestepped by providing cmake with the `-DCMAKE_C_COMPILER_WORKS=1` flag.
