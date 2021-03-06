/**
 * Marlin 3D Printer Firmware
 *
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 * Copyright (c) 2016 Bob Cousins bobcousins42@googlemail.com
 * Copyright (c) 2015-2016 Nico Tonnhofer wurstnase.reprap@gmail.com
 * Copyright (c) 2016 Victor Perez victor_pv@hotmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#if defined(STM32F7xx)

#include "../shared/persistent_store_api.h"

#include "../../inc/MarlinConfig.h"

#if ENABLED(EEPROM_SETTINGS)

#if DISABLED(EEPROM_EMULATED_WITH_SRAM)
#include <EEPROM.h>
#endif

bool PersistentStore::access_start() { return true; }
bool PersistentStore::access_finish() { return true; }

bool PersistentStore::write_data(int &pos, const uint8_t *value, size_t size, uint16_t *crc) {
  while (size--) {
    uint8_t v = *value;

    // Save to either program flash or Backup SRAM
    #if DISABLED(EEPROM_EMULATED_WITH_SRAM)
    // FLASH has only ~1,000 write cycles,
    // so only write bytes that have changed!
    if (v != EEPROM.read(pos)) {
      EEPROM.write(pos, v);
      if (EEPROM.read(pos) != v) {
        SERIAL_ECHO_START();
        SERIAL_ECHOLNPGM(MSG_ERR_EEPROM_WRITE);
        return true;
      }
    }    
    #else
    *(__IO uint8_t *) (BKPSRAM_BASE + (uint8_t * const)pos) = v;
    #endif

    crc16(crc, &v, 1);
    pos++;
    value++;
  };
  return false;
}

bool PersistentStore::read_data(int &pos, uint8_t* value, size_t size, uint16_t *crc, const bool writing) {
  do {
    // Read from either program flash or Backup SRAM
    #if DISABLED(EEPROM_EMULATED_WITH_SRAM)
    uint8_t c = EEPROM.read(pos);
    #else
    uint8_t c = (*(__IO uint8_t *) (BKPSRAM_BASE + ((unsigned char*)pos)));
    #endif

    if (writing) *value = c;
    crc16(crc, &c, 1);
    pos++;
    value++;
  } while (--size);
  return false;
}

size_t PersistentStore::capacity() {
	#if DISABLED(EEPROM_EMULATED_WITH_SRAM)
	return E2END + 1;
	#else
	return ((size_t)(4*1024))+1; //4kB
	#endif
}

#endif // EEPROM_SETTINGS
#endif // STM32F7xx
