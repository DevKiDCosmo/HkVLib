#include "nvs.h"

#include <Preferences.h>
#include "nvs_flash.h"
#include "esp_err.h"
#include "../serial/log.h"

namespace
{
    static const char *TAG = "NVS";
    Preferences g_preferences;
    bool g_flashInitialized = false;
    bool g_sessionOpen = false;
    String g_namespace = "hkv";
}

bool NVSStore::initializeFlash()
{
    if (g_flashInitialized)
    {
        return true;
    }

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        Log::sys_warning(TAG, "NVS partition re-init required");
        err = nvs_flash_erase();
        if (err != ESP_OK)
        {
            Log::sys_error(TAG, "nvs_flash_erase failed: " + String(esp_err_to_name(err)));
            return false;
        }

        err = nvs_flash_init();
    }

    if (err != ESP_OK)
    {
        Log::sys_error(TAG, "nvs_flash_init failed: " + String(esp_err_to_name(err)));
        return false;
    }

    g_flashInitialized = true;
    Log::sys_info(TAG, "NVS flash initialized");
    return true;
}

bool NVSStore::begin(const char *nameSpace, bool readOnly)
{
    if (nameSpace == nullptr || *nameSpace == '\0')
    {
        Log::sys_error(TAG, "Invalid namespace");
        return false;
    }

    if (!initializeFlash())
    {
        return false;
    }

    if (g_sessionOpen)
    {
        g_preferences.end();
        g_sessionOpen = false;
    }

    if (!g_preferences.begin(nameSpace, readOnly))
    {
        Log::sys_error(TAG, "Failed to open namespace: " + String(nameSpace));
        return false;
    }

    g_namespace = nameSpace;
    g_sessionOpen = true;
    return true;
}

void NVSStore::end()
{
    if (g_sessionOpen)
    {
        g_preferences.end();
        g_sessionOpen = false;
    }
}

bool NVSStore::ensureSession()
{
    if (g_sessionOpen)
    {
        return true;
    }

    return begin(g_namespace.c_str(), false);
}

const char *NVSStore::resolveKey(NVSKey::Key key)
{
    switch (key)
    {
    case NVSKey::Key::ServerSha256:
        return "srv_sha256";
    case NVSKey::Key::ConfigSha256:
        return "cfg_sha256";
    case NVSKey::Key::DevicePassword:
        return "device_pw";
    case NVSKey::Key::UnitTestDone:
        return "ut_done";
    case NVSKey::Key::LastUnitTestMs:
        return "ut_last_ms";
    case NVSKey::Key::LastRequiredUnitTest:
        return "lastRUT";
    default:
        return nullptr;
    }
}

bool NVSStore::setString(NVSKey::Key key, const String &value)
{
    if (!ensureSession())
    {
        return false;
    }

    const char *storageKey = resolveKey(key);
    if (storageKey == nullptr)
    {
        return false;
    }

    return g_preferences.putString(storageKey, value) > 0;
}

String NVSStore::getString(NVSKey::Key key, const String &fallback)
{
    if (!ensureSession())
    {
        return fallback;
    }

    const char *storageKey = resolveKey(key);
    if (storageKey == nullptr)
    {
        return fallback;
    }

    return g_preferences.getString(storageKey, fallback);
}

bool NVSStore::setBool(NVSKey::Key key, bool value)
{
    if (!ensureSession())
    {
        return false;
    }

    const char *storageKey = resolveKey(key);
    if (storageKey == nullptr)
    {
        return false;
    }

    return g_preferences.putBool(storageKey, value);
}

bool NVSStore::getBool(NVSKey::Key key, bool fallback)
{
    if (!ensureSession())
    {
        return fallback;
    }

    const char *storageKey = resolveKey(key);
    if (storageKey == nullptr)
    {
        return fallback;
    }

    return g_preferences.getBool(storageKey, fallback);
}

bool NVSStore::setUInt(NVSKey::Key key, std::uint32_t value)
{
    if (!ensureSession())
    {
        return false;
    }

    const char *storageKey = resolveKey(key);
    if (storageKey == nullptr)
    {
        return false;
    }

    return g_preferences.putUInt(storageKey, value) > 0;
}

std::uint32_t NVSStore::getUInt(NVSKey::Key key, std::uint32_t fallback)
{
    if (!ensureSession())
    {
        return fallback;
    }

    const char *storageKey = resolveKey(key);
    if (storageKey == nullptr)
    {
        return fallback;
    }

    return g_preferences.getUInt(storageKey, fallback);
}

bool NVSStore::removeKey(NVSKey::Key key)
{
    if (!ensureSession())
    {
        return false;
    }

    const char *storageKey = resolveKey(key);
    if (storageKey == nullptr)
    {
        return false;
    }

    return g_preferences.remove(storageKey);
}

void NVSStore::clear()
{
    if (!ensureSession())
    {
        return;
    }

    g_preferences.clear();
}
