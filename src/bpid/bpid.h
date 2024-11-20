/*
    This file is part of the Arduino_CloudUtils library.

    Copyright (c) 2024 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <Arduino.h>
#include "ucid.h"
#include "mac.h"
#include "csn.h"

namespace arduino { namespace bpid {
    /*
     * This library contains the methods to get board provisioning id
     */

    constexpr int BOARD_PROVISIONING_ID_SIZE  = arduino::ucid::UC_UID_SIZE +
                                                arduino::mac::IFACE_MAC_SIZE +
                                                arduino::csn::CRYPTO_SN_SIZE;

    bool get(uint8_t* in, uint32_t size);
    String get();

}} // arduino::bpid
