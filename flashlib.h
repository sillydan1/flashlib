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
#define FLASH_PROTECTED_ERR 0x8
#define EEPROM_OUT_OF_RANGE_ERR 0x9
#define FLASH_NOT_ALIGNED 0x10

#ifdef ENABLE_EEPROM_EMU
/// Read a word from provided eeprom address
/// Returns EEPROM_OUT_OF_RANGE_ERR if provided address is not in the eeprom sector
flword_t eeprom_read_word(flword_t* ee_address);

/// Write a single word to provided eeprom address
/// Returns EEPROM_OUT_OF_RANGE_ERR if provided address is not in the eeprom sector
flstatus_t eeprom_write_word(fladdr_t ee_address, flword_t data_word);
#endif

/// Read a single word from provided program memory address.
/// No out-of-bounds check is performed, so be careful.
/// TODO: What happens if read from nonexistent address - garbage data? Or reset trigger? - Undefined behavior perhaps?
flword_t flash_read_word(fladdr_t* address);

/// Write a single word to provided program memory address.
/// Returns FLASH_PROTECTED_ERR if address is in the protected sector
/// Returns 0 (nil) if successful
flstatus_t flash_write_word(fladdr_t address, flword_t data_word);

/// Write a single byte to provided program memory address.
/// Note: This is a read/modify/write function
/// Returns FLASH_PROTECTED_ERR if address is in the protected sector
/// Returns 0 (nil) if successful
flstatus_t flash_write_byte(fladdr_t address, uint8_t byte);

#ifdef ENABLE_DOUBLEWORD_PROGRAMMING
/// Write a double word to provided program memory address.
/// Returns 0 (nil) if successful
unsigned int flash_write_doubleword(void* address, unsigned int word_h, unsigned int word_l);
#endif

/// Write a row of data to provided program memory address.
/// Returns FLASH_PROTECTED_ERR if address is in the protected sector
/// Returns 0 (nil) if successful.
flstatus_t flash_write_row(fladdr_t address, const flword_t* data);

/// Erase a page in program memory.
/// Returns FLASH_PROTECTED_ERR if address is in the protected sector
/// Returns 0 (nil) if successful
flstatus_t flash_erase_page(fladdr_t address);

/// Writes a page in program memory.
/// Note: provided address should be page-aligned
/// Note: overrides any data that already exists on the page
/// Note: provided data array is assumed to be PAGE_SIZE of length
/// Returns FLASH_PROTECTED_ERR if address is in the protected sector
/// Returns FLASH_NOT_ALIGNED if address is not page-aligned
/// Returns 0 (nil) if successful
flstatus_t flash_write_page(fladdr_t address, const flword_t* data);

/// Read / Modify / Write cycle for programming a page with data with size less than a page
/// Note: provided address should be page-aligned
/// Note: if data_size is larger than PAGE_SIZE, only PAGE_SIZE amount of data is written. Rest is ignored
/// Returns FLASH_PROTECTED_ERR if address is in the protected sector
/// Returns FLASH_NOT_ALIGNED if address is not page-aligned
/// Returns 0 (nil) if successful
flstatus_t flash_program_page(fladdr_t address, const flword_t* data, flword_t data_size);

#ifndef DISABLE_ERASE_ALL_PROGRAM_MEM
/// Erase all data in program memory.
/// CAUTION: This will erase all of your program data. Including emulated EEPROM data.
flstatus_t flash_erase_all_program_memory();
#endif

#endif //FLASHLIB_H
