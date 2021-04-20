cmake_minimum_required(VERSION 3.7.0)
#   Flash programming and EEPROM emulation utility library for PIC32MX devices
#   Copyright (C) 2021 HMK Bilcon A/S
#   Author(s): Asger Gitz-Johansen
#
#   This file is part of flashlib.
#
#    flashlib is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    flashlib is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public License
#    along with flashlib.  If not, see <https://www.gnu.org/licenses/>.
project(flashlib)
set(DEFS)

## Enable eeprom emulation
if(DEFINED ENABLE_EEPROM_EMU)
    set(DEFS ${DEFS} ENABLE_EEPROM_EMU)
endif()
## Emulated EEPROM sector. Writes and reads from outside this sector results in nil data
if(DEFINED EEPROM_SECTOR_START)
    set(DEFS ${DEFS} EEPROM_SECTOR_START=${EEPROM_SECTOR_START})
endif()
if(DEFINED EEPROM_SECTOR_END)
    set(DEFS ${DEFS} EEPROM_SECTOR_END=${EEPROM_SECTOR_END})
endif()
## Enable wide word programming. Default is disabled
if(DEFINED ENABLE_DOUBLEWORD_PROGRAMMING)
    set(DEFS ${DEFS} ENABLE_DOUBLEWORD_PROGRAMMING)
endif()
## Protected flash sector (i.e. 0x1D000000 - 0x1D00FFFF). Default is no protection
if(DEFINED PROTECTED_FLASH_SECTOR_FROM)
    set(DEFS ${DEFS} PROTECTED_FLASH_SECTOR_FROM=${PROTECTED_FLASH_SECTOR_FROM})
endif()
if(DEFINED PROTECTED_FLASH_SECTOR_TO)
    set(DEFS ${DEFS} PROTECTED_FLASH_SECTOR_TO=${PROTECTED_FLASH_SECTOR_TO})
endif()

add_compile_definitions(${DEFS})
add_library(${PROJECT_NAME} flashlib.c)
