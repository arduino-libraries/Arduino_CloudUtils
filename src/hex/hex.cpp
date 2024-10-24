/*
    This file is part of the Arduino_CloudUtils library.

    Copyright (c) 2024 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "hex.h"

namespace arduino { namespace hex {

    String encode(uint8_t* in, uint32_t size) {
        String out;
        out.reserve((size * 2) + 1);

        char *ptr = out.begin();
        for (uint32_t i = 0; i < size; i++) {
            ptr += sprintf(ptr, "%02X", in[i]);
        }
        return String(out.c_str());
    }

}} // arduino::hex
