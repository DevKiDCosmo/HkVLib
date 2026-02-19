#pragma once
#include "mbedtls/md5.h"
#include "mbedtls/sha256.h"
#include <Arduino.h>

class Utility
{
public:
    static String wlan_lock(String ssid, String pw);
    static String toHex(const uint8_t *data, size_t len);
};