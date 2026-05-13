#include "./tick_accuracy.h"

#include <cstdint>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosTickAccuracyTest()
    {
        constexpr const char *kTag = "RTOS_TIME_TICK";
        constexpr std::int64_t kToleranceUs = 35000;

        const TickType_t nearWrap = static_cast<TickType_t>(~static_cast<TickType_t>(0)) - static_cast<TickType_t>(5u);
        const TickType_t wrapped = static_cast<TickType_t>(nearWrap + static_cast<TickType_t>(10u));
        const TickType_t wrappedDelta = wrapped - nearWrap;
        if (wrappedDelta != static_cast<TickType_t>(10u))
        {
            Log::sys_error(kTag, "Tick wraparound delta arithmetic failed");
            return false;
        }

        std::int64_t maxDrift = 0;
        for (int sample = 0; sample < 3; ++sample)
        {
            const TickType_t startTick = xTaskGetTickCount();
            const std::int64_t startUs = esp_timer_get_time();

            vTaskDelay(pdMS_TO_TICKS(150));

            const TickType_t endTick = xTaskGetTickCount();
            const std::int64_t endUs = esp_timer_get_time();

            const std::int64_t elapsedUs = endUs - startUs;
            const std::int64_t expectedUs = static_cast<std::int64_t>(endTick - startTick) * portTICK_PERIOD_MS * 1000LL;
            const std::int64_t drift = (elapsedUs >= expectedUs) ? (elapsedUs - expectedUs) : (expectedUs - elapsedUs);

            if (drift > maxDrift)
            {
                maxDrift = drift;
            }
        }

        if (maxDrift > kToleranceUs)
        {
            Log::sys_error(kTag, "Tick drift too high: " + String(static_cast<long long>(maxDrift)) + " us");
            return false;
        }

        Log::sys_info(kTag, "Tick accuracy successful, max-drift=" + String(static_cast<long long>(maxDrift)) + " us");
        return true;
    }
} // namespace UnitTest
