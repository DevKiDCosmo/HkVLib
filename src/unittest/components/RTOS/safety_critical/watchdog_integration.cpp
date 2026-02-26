#include "./watchdog_integration.h"

#include "esp_err.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosWatchdogIntegrationTest()
    {
        constexpr const char *kTag = "RTOS_SAFE_WDT";

        const esp_err_t status = esp_task_wdt_status(nullptr);
        if (status == ESP_OK || status == ESP_ERR_NOT_FOUND)
        {
            Log::sys_info(kTag, "Watchdog integration API reachable, status=" + String(static_cast<int>(status)));
            return true;
        }

        Log::sys_error(kTag, "Unexpected watchdog status: " + String(static_cast<int>(status)));
        return false;
    }
} // namespace UnitTest
