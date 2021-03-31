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

// TODO: This is for physical address space in flash memory.
unsigned char is_address_within_eeprom_sector(void* ee_physical_address) {
    return (unsigned int)ee_physical_address > EEPROM_SECTOR_START &&
           (unsigned int)ee_physical_address < EEPROM_SECTOR_END;
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
