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
#include <xc.h>
#include "flashlib.h"
#include "flash_operations.h"
#define NVMCONERRs_bitmask 0x3000; // LVDERR | WRERR
unsigned int flash_commit();
/// NOTE: On PIC32MX1XX/2XX/5XX 64/100-pin devices
/// the Flash page size is 1 KB and the row size is 128 bytes (256 IW and32IW, respectively).

#ifdef ENABLE_EEPROM_EMU
#ifndef EEPROM_SECTOR_START
#error "EEPROM_SECTOR_START is not defined. Provide a physical program flash address for this"
#endif
#ifndef EEPROM_SECTOR_END
#error "EEPROM_SECTOR_END is not defined. Provide a physical program flash address for this"
#endif
#if defined(PROTECTED_FLASH_SECTOR_FROM) && !defined(PROTECTED_FLASH_SECTOR_TO)
#error "When defining PROTECTED_FLASH_SECTOR_FROM you should also provide PROTECTED_FLASH_SECTOR_TO"
#endif
#if defined(PROTECTED_FLASH_SECTOR_TO) && !defined(PROTECTED_FLASH_SECTOR_FROM)
#error "When defining PROTECTED_FLASH_SECTOR_TO you should also provide PROTECTED_FLASH_SECTOR_FROM"
#endif

// TODO: This is for physical address space in flash memory.
unsigned char is_address_within_eeprom_sector(void* ee_kseg_address) {
    return (unsigned int)ee_kseg_address > EEPROM_SECTOR_START &&
           (unsigned int)ee_kseg_address < EEPROM_SECTOR_END;
}

unsigned int eeprom_read_word(void* ee_address) {
    if(!is_address_within_eeprom_sector(ee_address))
        return 0;
    return *(unsigned int*)ee_address;
}

unsigned int eeprom_write_word(void* ee_address, unsigned int data_word) {
    if(!is_address_within_eeprom_sector(ee_address))
        return 1;
    NVMADDR = (unsigned int)ee_address;
    NVMDATA = data_word;
    NVMCONbits.NVMOP = ProgramWord;
    return flash_commit();
}
#endif

unsigned char is_address_within_protected_sector(void* kseg_address) {
#if defined(PROTECTED_FLASH_SECTOR_FROM) && defined(PROTECTED_FLASH_SECTOR_TO)
    return  (unsigned int)kseg_address > PROTECTED_FLASH_SECTOR_FROM &&
            (unsigned int)kseg_address < PROTECTED_FLASH_SECTOR_TO;
#else
    return 0;
#endif
}

unsigned int flash_read_word(void* address) {
    if(is_address_within_protected_sector(address))
        return 0;
    return *(unsigned int*)address;
}

unsigned int flash_write_word(void* address, unsigned int data_word) {
    if(is_address_within_protected_sector(address))
        return 0;
    NVMADDR = (unsigned int)address;
    NVMDATA = data_word;
    NVMCONbits.NVMOP = ProgramWord;
    return flash_commit();
}

#ifdef ENABLE_DOUBLEWORD_PROGRAMMING
unsigned int flash_write_doubleword(void* address, unsigned int word_h, unsigned int word_l) {
    if(is_address_within_protected_sector(address))
        return 0;
    NVMADDR = (unsigned int)address;
    NVMDATA0 = word_l;
    NVMDATA1 = word_h;
    NVMCONbits.NVMOP = ProgramDoubleWord;
    return flash_commit();
}
#endif

unsigned int flash_write_row(void* address, void* data) {
    if(is_address_within_protected_sector(address))
        return 0;
    NVMADDR = (unsigned int) address;
    NVMSRCADDR = (unsigned int) data;
    NVMCONbits.NVMOP = ProgramRow;
    return flash_commit();
}

unsigned int flash_erase_page(void* address) {
    if(is_address_within_protected_sector(address))
        return 0;
    NVMADDR = (unsigned int) address;
    NVMCONbits.NVMOP = PageErase;
    return flash_commit();
}

unsigned int flash_erase_all_program_memory() {
    NVMCONbits.NVMOP = FlashFullErase;
    return flash_commit();
}

unsigned int flash_commit() {
    unsigned int status; // Disable interrupts
    asm volatile ("di %0" : "=r" (status));
    // Write to flash
    NVMCONbits.WREN = 1;
    NVMKEY = 0xAA996655;
    NVMKEY = 0x556699AA;
    NVMCONbits.WR = 1;
    while (NVMCONbits.WR);
    // Re-enable interrupts
    if (status & 0x00000001)
        asm volatile ("ei");
    else
        asm volatile ("di");
    NVMCONbits.WREN = 0;
    return NVMCON & NVMCONERRs_bitmask;
}
