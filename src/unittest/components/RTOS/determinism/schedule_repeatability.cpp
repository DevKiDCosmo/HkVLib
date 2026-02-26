#include "./schedule_repeatability.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosScheduleRepeatabilityTest()
    {
        constexpr const char *kTag = "RTOS_DET_SCH";
        constexpr int kSamples = 6;

        TickType_t baseline[kSamples]{};
        TickType_t verify[kSamples]{};

        TickType_t previous = xTaskGetTickCount();
        for (int i = 0; i < kSamples; ++i)
        {
            vTaskDelay(1);
            const TickType_t current = xTaskGetTickCount();
            baseline[i] = current - previous;
            previous = current;
        }

        previous = xTaskGetTickCount();
        for (int i = 0; i < kSamples; ++i)
        {
            vTaskDelay(1);
            const TickType_t current = xTaskGetTickCount();
            verify[i] = current - previous;
            previous = current;
        }

        for (int i = 0; i < kSamples; ++i)
        {
            const TickType_t a = baseline[i];
            const TickType_t b = verify[i];
            const TickType_t diff = (a > b) ? (a - b) : (b - a);
            if (diff > 1)
            {
                Log::sys_error(kTag, "Delta mismatch at index " + String(i) + ": base=" + String(a) + ", verify=" + String(b));
                return false;
            }
        }

        Log::sys_info(kTag, "Schedule repeatability successful");
        return true;
    }
} // namespace UnitTest
