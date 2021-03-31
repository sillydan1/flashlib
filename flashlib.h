/**
   Flash programming and EEPROM emulation utility library for PIC32MX devices
   Copyright (C) 2021 HMK Bilcon A/S
   Author(s): Asger Gitz-Johansen

   This file is part of flashlib.

    flashlib is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    flashlib is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with flashlib.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef FLASHLIB_H
#define FLASHLIB_H

#ifdef ENABLE_EEPROM_EMU
/// Read a word from provided eeprom address
/// Always returns 0 (nil) if provided address is out of bounds of the eeprom sector
unsigned int eeprom_read_word(void* ee_address);
/// Write a single word to provided eeprom address
/// Always returns 0 (nil) if provided address is out of bounds of the eeprom sector
unsigned int eeprom_write_word(void* ee_address, unsigned int data_word);
#endif
// FLASH page I/O


#endif //FLASHLIB_H
