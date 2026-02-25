#include "./timekeeping.h"

#include <cstdint>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosTimekeepingAfterSleepTest()
    {
        constexpr const char *kTag = "RTOS_PWR_TIME";

        const std::int64_t startUs = esp_timer_get_time();
        vTaskDelay(pdMS_TO_TICKS(60));
        const std::int64_t endUs = esp_timer_get_time();

        const std::int64_t elapsedUs = endUs - startUs;
        if (elapsedUs < 40000)
        {
            Log::sys_error(kTag, "Timekeeping after sleep drift too low: " + String(static_cast<long long>(elapsedUs)) + " us");
            return false;
        }

        Log::sys_info(kTag, "Timekeeping after sleep baseline successful");
        return true;
    }
} // namespace UnitTest
