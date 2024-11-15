/*
    This file is part of the Arduino_CloudUtils library.

    Copyright (c) 2024 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sha256.h"

#ifdef __cplusplus
}
#endif

class SHA256 {

public:

    static constexpr size_t HASH_SIZE = SHA256_SUM_LEN;

    void begin();
    void update(uint8_t const * data, size_t const len);
    void finalize(uint8_t * hash);

private:

    sha256_context _ctx;
};

namespace arduino { namespace sha256 {

    inline void begin(sha256_context * ctx) {
        return sha256_starts(ctx);
    }
    inline void update(sha256_context *ctx, const uint8_t *input, uint32_t length) {
        return sha256_update(ctx, input, length);
    }
    inline void finalize(sha256_context * ctx, uint8_t digest[SHA256_SUM_LEN]) {
        return sha256_finish(ctx, digest);
    }
    inline void oneshot(const unsigned char *input, unsigned int ilen,
		unsigned char *output, unsigned int chunk_sz) {
        return sha256_csum_wd(input, ilen, output, chunk_sz);
    }

}} // arduino::sha256
