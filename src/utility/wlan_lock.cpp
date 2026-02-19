#include "utility.h"
#include "mbedtls/md5.h"
#include "mbedtls/sha256.h"
#include "../serial/log.h"

static const char *TAG = "WLAN_LOCK";

String Utility::wlan_lock(String g_ssid, String g_password)
{
    // hash ssid+pw
    String Hash_WIFI;
    String creds = g_ssid + g_password;

    uint8_t md5[16];
    mbedtls_md5(reinterpret_cast<const unsigned char *>(creds.c_str()), creds.length(), md5);
    String hash_md5 = toHex(md5, sizeof(md5));

    uint8_t sha[32];
    mbedtls_sha256(reinterpret_cast<const unsigned char *>(creds.c_str()), creds.length(), sha, 0);
    String hash_sha256 = toHex(sha, sizeof(sha));

    Hash_WIFI = hash_sha256 + hash_md5;

    Log::sys_info(TAG, "WLAN_LOCK. MD5: " + hash_md5);
    Log::sys_info(TAG, "WLAN_LOCK. SHA256: " + hash_sha256);
    return Hash_WIFI;
}