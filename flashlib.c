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
#include <sys/kmem.h>
#include "flashlib.h"
#include "flash_operations.h"
#define NVMCONERRs_bitmask ( _NVMCON_WRERR_MASK | _NVMCON_LVDERR_MASK )
#define NVMCON_clearmask ( _NVMCON_NVMOP_MASK | _NVMCON_WRERR_MASK | _NVMCON_LVDERR_MASK )
flstatus_t flash_commit(flash_operation_t op);
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
unsigned char is_address_within_eeprom_sector(fladdr_t ee_kseg_address) {
    return ee_kseg_address >= EEPROM_SECTOR_START &&
           ee_kseg_address <= EEPROM_SECTOR_END;
}

flword_t eeprom_read_word(fladdr_t* ee_address) {
    if(!is_address_within_eeprom_sector((fladdr_t)ee_address))
        return EEPROM_OUT_OF_RANGE_ERR;
    return *ee_address;
}

flstatus_t eeprom_write_word(fladdr_t ee_address, flword_t data_word) {
    if(!is_address_within_eeprom_sector(ee_address))
        return EEPROM_OUT_OF_RANGE_ERR;
    NVMDATA = data_word;
    NVMADDR = KVA_TO_PA(ee_address);
    return flash_commit(ProgramWord);
}
#endif

unsigned char is_address_within_protected_sector(fladdr_t phys_address) {
#if defined(PROTECTED_FLASH_SECTOR_FROM) && defined(PROTECTED_FLASH_SECTOR_TO)
    return KVA_TO_PA(phys_address) > KVA_TO_PA(PROTECTED_FLASH_SECTOR_FROM) &&
           KVA_TO_PA(phys_address) < KVA_TO_PA(PROTECTED_FLASH_SECTOR_TO);
#else
    return 0;
#endif
}

flword_t flash_read_word(fladdr_t* address) {
    if(is_address_within_protected_sector((fladdr_t)address))
        return FLASH_PROTECTED_ERR;
    return *address;
}

flstatus_t flash_write_word(fladdr_t address, flword_t data_word) {
    if(is_address_within_protected_sector(address))
        return FLASH_PROTECTED_ERR;
    NVMDATA = data_word;
    NVMADDR = (fladdr_t) KVA_TO_PA(address);
    return flash_commit(ProgramWord);
}

flstatus_t flash_write_byte(fladdr_t address, uint8_t byte) {
    if(is_address_within_protected_sector(address))
        return FLASH_PROTECTED_ERR;
    const uint8_t offset = address % 4;
    const fladdr_t aligned_32bit_address = address - offset;
    flword_t data_at_address = flash_read_word((fladdr_t *)aligned_32bit_address);
    const flword_t byte_as_aligned_word = byte << offset*8;
    const flword_t aligned_mask = ~(0xFF << offset*8);
    data_at_address = (data_at_address & aligned_mask) | (byte_as_aligned_word);
    return flash_write_word(aligned_32bit_address, data_at_address);
}

#ifdef ENABLE_DOUBLEWORD_PROGRAMMING
flstatus_t flash_write_doubleword(fladdr_t address, flword_t word_h, flword_t word_l) {
    if(is_address_within_protected_sector(address))
        return FLASH_PROTECTED_ERR;
    NVMADDR = (fladdr_t) KVA_TO_PA(address);
    // If NVMDATA0, NVMDATA1 etc does not exist, your MCU might not support doubleword programming
    NVMDATA0 = word_l;
    NVMDATA1 = word_h;
    return flash_commit(ProgramDoubleWord);
}
#endif

flstatus_t flash_write_row(fladdr_t address, fladdr_t data_addr) {
    if(is_address_within_protected_sector(address))
        return FLASH_PROTECTED_ERR;
    NVMSRCADDR = (fladdr_t) KVA_TO_PA(data_addr);
    NVMADDR = KVA_TO_PA(address);
    return flash_commit(ProgramRow);
}

flstatus_t flash_erase_page(fladdr_t address) {
    if(is_address_within_protected_sector(address))
        return FLASH_PROTECTED_ERR;
    NVMADDR = (fladdr_t) KVA_TO_PA(address);
    return flash_commit(PageErase);
}

#ifndef DISABLE_ERASE_ALL_PROGRAM_MEM
//// This will erase all program memory. Use with caution
flstatus_t flash_erase_all_program_memory() {
    return flash_commit(FlashFullErase);
}
#endif

flstatus_t flash_commit(flash_operation_t op) {
    fladdr_t status;
    asm volatile ("di %0" : "=r" (status));

    NVMCONCLR = NVMCON_clearmask;
    NVMCONSET = op; // NVMCON_WREN is part of this
    NVMKEY = 0x00000000;
    NVMKEY = 0xAA996655;
    NVMKEY = 0x556699AA;
    NVMCONSET = NVMCON_WR;
    while (NVMCONbits.WR);

    // Re-enable interrupts
    if (status & 0x00000001)
        asm volatile ("ei");
    else
        asm volatile ("di");

    NVMCONCLR = NVMCON_WREN;
    return NVMCON & NVMCONERRs_bitmask;
}
