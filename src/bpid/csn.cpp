/*
    This file is part of the Arduino_CloudUtils library.

    Copyright (c) 2024 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "csn.h"

namespace arduino { namespace csn {

    bool get(uint8_t *in, uint32_t size) {
#if defined ARDUINO_UNOR4_WIFI
    return false;
#else
        if (size < CRYPTO_SN_SIZE) {
            return false;
        }
        SecureElement se;
        if (!se.begin() || !se.serialNumber(in)) {
            return false;
        }
        return true;
#endif
    }

}} // arduino::csn
