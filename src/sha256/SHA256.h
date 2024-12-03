/*
    This file is part of the Arduino_CloudUtils library.

    Copyright (c) 2024 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include "sha2.h"

class SHA256 {

public:

    static constexpr uint32_t HASH_SIZE = SHA256_DIGEST_SIZE;

    void begin();
    void update(uint8_t const * data, uint32_t const len);
    void finalize(uint8_t * hash);

private:

    sha256_ctx _ctx;
};

namespace arduino { namespace sha256 {

    inline void begin(sha256_ctx * ctx) {
        return sha256_init(ctx);
    }
    inline void update(sha256_ctx *ctx, const uint8_t *input, uint32_t length) {
        return sha256_update(ctx, input, length);
    }
    inline void finalize(sha256_ctx * ctx, uint8_t digest[SHA256_DIGEST_SIZE]) {
        return sha256_final(ctx, digest);
    }
    inline void sha256(const unsigned char *input, unsigned int ilen, unsigned char *output) {
        ::sha256(input, ilen, output);
    }

}} // arduino::sha256
