#pragma once

#include <Arduino.h>
#include <cstdint>

namespace NVSKey
{
    enum class Key
    {
        ServerSha256,
        ConfigSha256,
        DevicePassword,
        UnitTestDone,
        LastUnitTestMs,
        LastRequiredUnitTest
    };
}

class NVSStore
{
public:
    static bool begin(const char *nameSpace = "hkv", bool readOnly = false);
    static void end();

    static bool setString(NVSKey::Key key, const String &value);
    static String getString(NVSKey::Key key, const String &fallback = "");

    static bool setBool(NVSKey::Key key, bool value);
    static bool getBool(NVSKey::Key key, bool fallback = false);

    static bool setUInt(NVSKey::Key key, std::uint32_t value);
    static std::uint32_t getUInt(NVSKey::Key key, std::uint32_t fallback = 0);

    static bool removeKey(NVSKey::Key key);
    static void clear();

private:
    static bool ensureSession();
    static bool initializeFlash();
    static const char *resolveKey(NVSKey::Key key);
};
