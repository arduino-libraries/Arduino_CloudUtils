/*
  This file is part of the Arduino_CloudUtils library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <Arduino_FlashFormatter.h>

FlashFormatter flashFormatter;

void setup() {
  Serial.begin(9600);
  while(!Serial);

  if(!flashFormatter.checkandFormatPartition()){
    Serial.println("Failed to format partition");
  } else {
    Serial.println("Partition formatted successfully");
  }
}

void loop() { }
