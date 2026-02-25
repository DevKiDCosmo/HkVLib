#include "./deadline.h"

#include <cstdint>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosTaskExecutionDeadlineTest()
    {
        constexpr const char *kTag = "RTOS_TIME_DEAD";
        constexpr std::int64_t kDeadlineUs = 20000;
        const TickType_t period = pdMS_TO_TICKS(25) > 0 ? pdMS_TO_TICKS(25) : 1;

        TickType_t wake = xTaskGetTickCount();
        std::uint32_t misses = 0u;

        for (int cycle = 0; cycle < 8; ++cycle)
        {
            const std::int64_t start = esp_timer_get_time();

            volatile std::uint32_t acc = 0u;
            for (std::uint32_t i = 0; i < 20000u; ++i)
            {
                acc ^= (i * 33u);
            }

            const std::int64_t elapsed = esp_timer_get_time() - start;
            if (elapsed > kDeadlineUs)
            {
                ++misses;
            }

            (void)acc;
            vTaskDelayUntil(&wake, period);
        }

        if (misses > 0u)
        {
            Log::sys_error(kTag, "Deadline miss detection triggered, misses=" + String(misses));
            return false;
        }

        Log::sys_info(kTag, "Task execution deadline successful");
        return true;
    }
} // namespace UnitTest
