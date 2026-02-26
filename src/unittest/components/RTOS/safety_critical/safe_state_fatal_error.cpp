#include "./safe_state_fatal_error.h"

#include "esp_err.h"
#include "esp_system.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        void shutdownHandler()
        {
        }
    } // namespace

    bool runRtosSafeStateOnFatalErrorTest()
    {
        constexpr const char *kTag = "RTOS_SAFE_FTL";

        const esp_err_t registerResult = esp_register_shutdown_handler(shutdownHandler);
        if (registerResult != ESP_OK)
        {
            Log::sys_error(kTag, "Failed to register shutdown handler: " + String(static_cast<int>(registerResult)));
            return false;
        }

        const esp_err_t unregisterResult = esp_unregister_shutdown_handler(shutdownHandler);
        if (unregisterResult != ESP_OK)
        {
            Log::sys_error(kTag, "Failed to unregister shutdown handler: " + String(static_cast<int>(unregisterResult)));
            return false;
        }

        Log::sys_info(kTag, "Safe state handling hooks available");
        return true;
    }
} // namespace UnitTest
