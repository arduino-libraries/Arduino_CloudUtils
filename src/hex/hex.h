/*
    This file is part of the Arduino_CloudUtils library.

    Copyright (c) 2024 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <Arduino.h>

namespace arduino { namespace hex {
    /*
     * This library contains the methods to get board provisioning id
     */

    String encode(uint8_t* in, uint32_t size);

}} // arduino::hex
