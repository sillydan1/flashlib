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
#include <string.h>
#include "flashlib.h"
#include "flash_operations.h"

flstatus_t flash_commit(flash_operation_t op);
#define NVMCONERRs_bitmask ( _NVMCON_WRERR_MASK | _NVMCON_LVDERR_MASK )
#define NVMCON_clearmask ( _NVMCON_NVMOP_MASK | _NVMCON_WRERR_MASK | _NVMCON_LVDERR_MASK )

/// NOTE: On PIC32MX1XX/2XX/5XX 64/100-pin devices
/// the Flash page size is 1 KiB and the row size is 128 bytes (256 IW and32IW, respectively).
#define PAGE_SIZE 0x400
#define ROW_SIZE 0x80
#define ROWS_IN_PAGE (PAGE_SIZE / ROW_SIZE)
#define WORDS_IN_ROW (ROW_SIZE / sizeof(flword_t))
#define WORDS_IN_PAGE (PAGE_SIZE / sizeof(flword_t))

#if defined(PROTECTED_FLASH_SECTOR_FROM) && !defined(PROTECTED_FLASH_SECTOR_TO)
    #error "When defining PROTECTED_FLASH_SECTOR_FROM you should also provide PROTECTED_FLASH_SECTOR_TO"
#endif
#if defined(PROTECTED_FLASH_SECTOR_TO) && !defined(PROTECTED_FLASH_SECTOR_FROM)
    #error "When defining PROTECTED_FLASH_SECTOR_TO you should also provide PROTECTED_FLASH_SECTOR_FROM"
#endif

unsigned char is_address_kseg(fladdr_t address) {
    // KSEG0 = 0x9xxxxxxx
    // KSEG1 = 0xBxxxxxxx
    return (address & 0xF0000000) >= 0x90000000;
}

#ifdef ENABLE_EEPROM_EMU
    #ifndef EEPROM_SECTOR_START
        #error "EEPROM_SECTOR_START is not defined. Provide a physical program flash address for this"
    #endif
    #ifndef EEPROM_SECTOR_END
        #error "EEPROM_SECTOR_END is not defined. Provide a physical program flash address for this"
    #endif
    #define ABS(N) (((N)<0)?(-(N)):(N))
    #define EEPROM_SECTOR_SIZE ABS((EEPROM_SECTOR_START - (EEPROM_SECTOR_END+1)))
    #if EEPROM_SECTOR_SIZE < PAGE_SIZE
        #error "EEPROM sector is smaller than device's page size"
    #endif
    #if ((EEPROM_SECTOR_SIZE % PAGE_SIZE) != 0) || (EEPROM_SECTOR_START % PAGE_SIZE != 0)
        #error "EEPROM sector is not aligned with page size"
    #endif

    unsigned char is_address_within_eeprom_sector(fladdr_t address) {
        return  KVA_TO_PA(address) >= KVA_TO_PA(EEPROM_SECTOR_START) &&
                KVA_TO_PA(address) <= KVA_TO_PA(EEPROM_SECTOR_END);
    }

    flword_t eeprom_read_word(flword_t* ee_address) {
        if(!is_address_kseg((fladdr_t)ee_address))
            return FLASH_ADDRESS_NOT_KERNEL_SPACE;
        if(!is_address_within_eeprom_sector((fladdr_t)ee_address))
            return EEPROM_OUT_OF_RANGE_ERR;
        return *ee_address;
    }

    flstatus_t eeprom_write_word(fladdr_t ee_address, flword_t data_word) {
        if(!is_address_within_eeprom_sector(ee_address))
            return EEPROM_OUT_OF_RANGE_ERR;
        return flash_program_page_offset(ee_address, &data_word, sizeof(flword_t));
    }

#endif // ENABLE_EEPROM_EMU

unsigned char is_address_within_protected_sector(fladdr_t address) {
#if defined(PROTECTED_FLASH_SECTOR_FROM) && defined(PROTECTED_FLASH_SECTOR_TO)
    return KVA_TO_PA(address) >= KVA_TO_PA(PROTECTED_FLASH_SECTOR_FROM) &&
           KVA_TO_PA(address) <= KVA_TO_PA(PROTECTED_FLASH_SECTOR_TO);
#else
    return 1;
#endif
}

flstatus_t flash_program_page(fladdr_t kseg1_address, const flword_t* data, flword_t data_byte_size) {
    if(!is_address_kseg(kseg1_address))
        return FLASH_ADDRESS_NOT_KERNEL_SPACE;
    if(is_address_within_protected_sector(kseg1_address))
        return FLASH_PROTECTED_ERR;
    if(kseg1_address % PAGE_SIZE != 0)
        return FLASH_NOT_ALIGNED;

    flword_t flash_page[PAGE_SIZE];
    memcpy(flash_page, (flword_t*)kseg1_address, PAGE_SIZE);
    memcpy(flash_page, data, data_byte_size);
    return flash_write_page(kseg1_address, flash_page);
}

flstatus_t flash_program_page_offset(fladdr_t kseg1_address, const flword_t* data, flword_t data_byte_size) {
    if(!is_address_kseg(kseg1_address))
        return FLASH_ADDRESS_NOT_KERNEL_SPACE;
    flword_t offset = kseg1_address % PAGE_SIZE;
    fladdr_t offset_address = kseg1_address - offset;
    if(is_address_within_protected_sector(offset_address))
        return FLASH_PROTECTED_ERR;

    uint8_t flash_page[PAGE_SIZE];
    memcpy(flash_page, (uint8_t*)offset_address, PAGE_SIZE);
    memcpy(&flash_page[offset], (const uint8_t*)data, min(data_byte_size, (PAGE_SIZE - offset)));
    return flash_write_page(offset_address, (const flword_t *) flash_page);
}

flstatus_t flash_write_page(fladdr_t address, const flword_t* data) {
    if(is_address_within_protected_sector(address))
        return FLASH_PROTECTED_ERR;
    if(address % PAGE_SIZE != 0)
        return FLASH_NOT_ALIGNED;

    flstatus_t status = flash_erase_page(address);
    if(status != 0)
        return status;

    uint16_t i = 0;
    for(uint16_t row = 0; row < ROWS_IN_PAGE; row++) {
        status = flash_write_row(address, &data[i]);
        i += WORDS_IN_ROW;
        address += ROW_SIZE;
        if(status != 0)
            return status;
    }
    return status;
}

flstatus_t flash_erase_page(fladdr_t address) {
    if(is_address_within_protected_sector(address))
        return FLASH_PROTECTED_ERR;
    if(address % PAGE_SIZE != 0)
        return FLASH_NOT_ALIGNED;
    NVMADDR = (fladdr_t) KVA_TO_PA(address);
    return flash_commit(PageErase);
}

flstatus_t flash_write_row(fladdr_t address, const flword_t* data) {
    if(is_address_within_protected_sector(address))
        return FLASH_PROTECTED_ERR;
    NVMSRCADDR = KVA_TO_PA((fladdr_t)data);
    NVMADDR = KVA_TO_PA(address);
    return flash_commit(ProgramRow);
}

flstatus_t flash_write_word(fladdr_t address, flword_t data_word) {
    return flash_program_page_offset(address, &data_word, sizeof(flword_t));
}

#ifndef DISABLE_ERASE_ALL_PROGRAM_MEM
//// This will erase all program memory. Use with caution
flstatus_t flash_erase_all_program_memory() {
    return flash_commit(FlashFullErase);
}
#endif

flstatus_t flash_commit(flash_operation_t op) {
    // Disable interrupts
    fladdr_t status = __builtin_disable_interrupts();

    NVMCONCLR = NVMCON_clearmask;
    NVMCONSET = op; // NVMCON_WREN is part of this
    NVMKEY = 0x00000000;
    NVMKEY = 0xAA996655;
    NVMKEY = 0x556699AA;
    NVMCONSET = NVMCON_WR;
    while (NVMCONbits.WR);

    // Re-enable interrupts
    __builtin_mtc0(12, 0, status);

    NVMCONCLR = NVMCON_WREN;
    return NVMCON & NVMCONERRs_bitmask;
}
