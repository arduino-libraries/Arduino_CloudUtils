/*
    This file is part of the Arduino_CloudUtils library.

    Copyright (c) 2024 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <Arduino_SHA256.h>
#include "buffer.h"

void setup() {
    Serial.begin(9600);
    while(!Serial);

    uint8_t sha[SHA256::HASH_SIZE];

    SHA256 sha256;
    sha256.begin();
    sha256.update(buffer, sizeof(buffer));
    sha256.finalize(sha);

    Serial.println(hexEncode(sha, sizeof(sha)));

    /* One-shot */
    arduino::sha256::sha256(buffer, sizeof(buffer), sha);
    Serial.println(hexEncode(sha, sizeof(sha)));
}

static String hexEncode(uint8_t* in, uint32_t size) {
  String out;
  out.reserve((size * 2) + 1);

  char* ptr = out.begin();
  for (uint32_t i = 0; i < size; i++) {
    ptr += sprintf(ptr, "%02X", in[i]);
  }
  return String(out.c_str());
}

void loop() { }
