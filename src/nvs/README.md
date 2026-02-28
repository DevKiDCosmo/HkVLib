# NVS module

This module provides persistent key-value storage for runtime and security related values using ESP32 NVS.

## Stored values

- `NVSKey::Key::ServerSha256` -> `srv_sha256`
- `NVSKey::Key::ConfigSha256` -> `cfg_sha256`
- `NVSKey::Key::DevicePassword` -> `device_pw`
- `NVSKey::Key::UnitTestDone` -> `ut_done`
- `NVSKey::Key::LastUnitTestMs` -> `ut_last_ms`
- `NVSKey::Key::LastRequiredUnitTest` -> `lastRUT`

## API

- Header: `src/nvs/nvs.h`
- Implementation: `src/nvs/nvs.cpp`
- Main class: `NVSStore`

Typical workflow:

1. `NVSStore::begin()` at startup
2. read/write values with typed functions and enum keys
3. `NVSStore::end()` when session can be closed

Example:

- write string: `NVSStore::setString(NVSKey::Key::DevicePassword, "secret")`
- read string: `NVSStore::getString(NVSKey::Key::DevicePassword)`
- write bool: `NVSStore::setBool(NVSKey::Key::UnitTestDone, true)`