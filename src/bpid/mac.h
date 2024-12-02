/*
    This file is part of the Arduino_CloudUtils library.

    Copyright (c) 2024 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <Arduino.h>

#if   defined(ARDUINO_NANO_RP2040_CONNECT) || \
      defined(ARDUINO_SAMD_MKRWIFI1010)    || \
      defined(ARDUINO_SAMD_NANO_33_IOT)    || \
      defined(ARDUINO_PORTENTA_H7_M7)      || \
      defined(ARDUINO_NICLA_VISION)        || \
      defined(ARDUINO_GIGA)
    #include <WiFi.h>
    #define IFACE_MAC_ADDR_LENGTH WL_MAC_ADDR_LENGTH
#elif defined(ARDUINO_PORTENTA_C33)
    #include <WiFiC3.h>
    #define IFACE_MAC_ADDR_LENGTH WL_MAC_ADDR_LENGTH
#elif defined(ARDUINO_UNOR4_WIFI)
    #include <WiFi.h>
    #define IFACE_MAC_ADDR_LENGTH 6
#elif defined(ARDUINO_OPTA)
    #include <Ethernet.h>
    #define IFACE_MAC_ADDR_LENGTH 6
#else
    #define IFACE_MAC_ADDR_LENGTH 0
#endif

namespace arduino { namespace mac {
    /*
     * This library contains the methods to get board mac address
     * ARDUINO_NANO_RP2040_CONNECT: reversed
     * ARDUINO_SAMD_MKRWIFI1010: reversed
     * ARDUINO_SAMD_NANO_33_IOT: reversed
     * ARDUINO_PORTENTA_H7_M7: WiFi.setTimeout(0);WiFi.begin("In33dm4c4ddr35", "In33dm4c4ddr35", ENC_TYPE_TKIP) reversed
     * ARDUINO_NICLA_VISION:
     * ARDUINO_GIGA:
     * ARDUINO_PORTENTA_C33: Interface.begin() not reversed
     * ARDUINO_UNOR4_WIFI: not reversed
     * ARDUINO_OPTA: Ethernet.begin(NULL,0,0); reversed
     */

    bool get(uint8_t *in, uint32_t size);

}} // arduino::mac
