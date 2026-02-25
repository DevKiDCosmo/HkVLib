#include "./timing.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../serial/log.h"

namespace UnitTest
{
    bool runRtosTimingTest()
    {
        constexpr const char *kTag = "RTOS_TIME";
        const TickType_t period = pdMS_TO_TICKS(10) > 0 ? pdMS_TO_TICKS(10) : 1;

        TickType_t wake = xTaskGetTickCount();
        TickType_t previous = wake;

        for (int i = 0; i < 5; ++i)
        {
            vTaskDelayUntil(&wake, period);
            const TickType_t now = xTaskGetTickCount();
            const TickType_t delta = now - previous;

            if (delta < period || delta > (period + 1))
            {
                Log::sys_error(kTag, "Period violation at cycle " + String(i) + ": " + String(delta));
                return false;
            }

            previous = now;
        }

        Log::sys_info(kTag, "Timing guarantees baseline successful");
        return true;
    }
} // namespace UnitTest
