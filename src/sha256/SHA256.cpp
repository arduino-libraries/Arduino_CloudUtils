/*
    This file is part of the Arduino_CloudUtils library.

    Copyright (c) 2024 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "SHA256.h"

void SHA256::begin() {
    sha256_init(&_ctx);
}

void SHA256::update(uint8_t const * data, uint32_t const len) {
    sha256_update(&_ctx, data, len);
}

void SHA256::finalize(uint8_t * hash) {
    sha256_final(&_ctx, hash);
}
