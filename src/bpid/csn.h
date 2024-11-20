/*
    This file is part of the Arduino_CloudUtils library.

    Copyright (c) 2024 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <Arduino.h>
#include <Arduino_SecureElement.h>

namespace arduino { namespace csn {
    /*
     * This library contains the methods to get board microcontroller id
     */

#if   defined(ARDUINO_NANO_RP2040_CONNECT) || \
      defined(ARDUINO_SAMD_MKRWIFI1010)    || \
      defined(ARDUINO_SAMD_NANO_33_IOT)    || \
      defined(ARDUINO_PORTENTA_H7_M7)      || \
      defined(ARDUINO_OPTA)                || \
      defined(ARDUINO_GIGA)
    constexpr int CRYPTO_SN_SIZE  = 9;
#elif defined(ARDUINO_PORTENTA_C33)        || \
      defined(ARDUINO_NICLA_VISION)
    constexpr int CRYPTO_SN_SIZE  = SE05X_SN_LENGTH;
#elif defined(ARDUINO_UNOR4_WIFI)
    constexpr int CRYPTO_SN_SIZE  = 6;
#else
    #error "Unknown board"
#endif

    bool get(uint8_t *in, uint32_t size);

}} // arduino::csn
