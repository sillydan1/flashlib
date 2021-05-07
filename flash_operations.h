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
#ifndef FLASH_OPERATIONS_H
#define FLASH_OPERATIONS_H

/// Documentation: DS60001121 - Register 5-1
typedef enum {
    NOP                 = NVMCON_WREN + 0x0000, // No operation
    FlashFullErase      = NVMCON_WREN + 0x0005, // Program Flash Memory Erase Operation - For self destruction
    PageErase           = NVMCON_WREN + 0x0004, // Erases page selected by NVMADDR
    ProgramRow          = NVMCON_WREN + 0x0003, // Programs row selected by NVMSRCADDR (Supported by all devices)
    ProgramDoubleWord   = NVMCON_WREN + 0x0010, // Programs double-word selected by NVMADDR (Supported by select devices)
    ProgramWord         = NVMCON_WREN + 0x0001  // Programs word selected by NVMADDR (Supported by most devices)
} flash_operation_t;

#endif //FLASH_OPERATIONS_H
