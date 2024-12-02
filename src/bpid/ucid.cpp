/*
    This file is part of the Arduino_CloudUtils library.

    Copyright (c) 2024 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "ucid.h"

namespace arduino { namespace ucid {

    bool get(uint8_t *in, uint32_t size) {
#if UC_UID_SIZE == 0
        (void)in;
        (void)size;
        return false;
#else
        if (size < UC_UID_SIZE) {
            return false;
        }
    #if   defined(ARDUINO_SAMD_MKRWIFI1010) || \
          defined(ARDUINO_SAMD_NANO_33_IOT)
        (*(uint32_t*)(&in[0x0])) = __builtin_bswap32(*(volatile uint32_t*)(0x0080A00C));
        (*(uint32_t*)(&in[0x4])) = __builtin_bswap32(*(volatile uint32_t*)(0x0080A040));
        (*(uint32_t*)(&in[0x8])) = __builtin_bswap32(*(volatile uint32_t*)(0x0080A044));
        (*(uint32_t*)(&in[0xC])) = __builtin_bswap32(*(volatile uint32_t*)(0x0080A048));
    #elif defined(ARDUINO_PORTENTA_H7_M7)   || \
          defined(ARDUINO_NICLA_VISION)     || \
          defined(ARDUINO_OPTA)             || \
          defined(ARDUINO_GIGA)
        (*(uint32_t*)(&in[0x0])) = __builtin_bswap32(HAL_GetUIDw0());
        (*(uint32_t*)(&in[0x4])) = __builtin_bswap32(HAL_GetUIDw1());
        (*(uint32_t*)(&in[0x8])) = __builtin_bswap32(HAL_GetUIDw2());
    #elif defined(ARDUINO_PORTENTA_C33)     || \
          defined(ARDUINO_UNOR4_WIFI)
        const bsp_unique_id_t* t = R_BSP_UniqueIdGet();
        (*(uint32_t*)(&in[0x0])) = __builtin_bswap32(t->unique_id_words[0x0]);
        (*(uint32_t*)(&in[0x4])) = __builtin_bswap32(t->unique_id_words[0x1]);
        (*(uint32_t*)(&in[0x8])) = __builtin_bswap32(t->unique_id_words[0x2]);
        (*(uint32_t*)(&in[0xC])) = __builtin_bswap32(t->unique_id_words[0x3]);
    #elif defined(ARDUINO_NANO_RP2040_CONNECT)
        uint8_t id[UC_UID_SIZE];
        flash_get_unique_id(id);
        (*(uint32_t*)(&in[0x0])) = __builtin_bswap32(*(uint32_t*)&id[0x0]);
        (*(uint32_t*)(&in[0x4])) = __builtin_bswap32(*(uint32_t*)&id[0x4]);
    #endif
        return true;
#endif
    }

}} // arduino::ucid
