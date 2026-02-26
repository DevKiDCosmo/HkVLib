#include "./timing_repeatability.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosTimingRepeatabilityTest()
    {
        constexpr const char *kTag = "RTOS_DET_TIM";
        constexpr int kRuns = 3;

        TickType_t durations[kRuns]{};
        for (int run = 0; run < kRuns; ++run)
        {
            const TickType_t start = xTaskGetTickCount();
            for (int i = 0; i < 8; ++i)
            {
                vTaskDelay(1);
            }
            durations[run] = xTaskGetTickCount() - start;
        }

        for (int run = 1; run < kRuns; ++run)
        {
            const TickType_t a = durations[run - 1];
            const TickType_t b = durations[run];
            const TickType_t diff = (a > b) ? (a - b) : (b - a);
            if (diff > 1)
            {
                Log::sys_error(kTag, "Run time jitter too high between runs " + String(run - 1) + " and " + String(run) + ": diff=" + String(diff));
                return false;
            }
        }

        Log::sys_info(kTag, "Timing repeatability successful");
        return true;
    }
} // namespace UnitTest
