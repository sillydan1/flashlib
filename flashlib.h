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
typedef uint32_t fladdr_t;
typedef uint32_t flword_t;
typedef uint16_t flstatus_t;

#ifdef ENABLE_EEPROM_EMU
/// Read a word from provided eeprom address
/// Always returns 0 (nil) if provided address is out of bounds of the eeprom sector
flword_t eeprom_read_word(fladdr_t* ee_address);

/// Write a single word to provided eeprom address
/// Always returns 1 (one) if provided address is out of bounds of the eeprom sector
flstatus_t eeprom_write_word(fladdr_t ee_address, flword_t data_word);
#endif

/// Read a single word from provided program memory address.
/// No out-of-bounds check is performed, so be careful.
/// TODO: What happens if read from nonexistent address - garbage data? Or reset trigger? - Undefined behavior perhaps?
flword_t flash_read_word(fladdr_t* address);

/// Write a single word to provided program memory address.
/// Returns 0 (nil) if successful
flstatus_t flash_write_word(fladdr_t address, flword_t data_word);

#ifdef ENABLE_DOUBLEWORD_PROGRAMMING
/// Write a double word to provided program memory address.
/// Returns 0 (nil) if successful
unsigned int flash_write_doubleword(void* address, unsigned int word_h, unsigned int word_l);
#endif

/// Write a row of data to provided program memory address.
/// Returns 0 (nil) if successful.
flstatus_t flash_write_row(fladdr_t address, fladdr_t data_addr);

/// Erase a page in program memory.
/// Returns 0 (nil) if successful
flstatus_t flash_erase_page(fladdr_t address);

/// Erase all data in program memory.
/// CAUTION: This will erase all of your program data. Including emulated EEPROM data.
/// Returns 0 (nil) if successful
flstatus_t flash_erase_all_program_memory();

#endif //FLASHLIB_H
