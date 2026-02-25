#include "./jitter.h"

#include <cstdint>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosJitterMeasurementTest()
    {
        constexpr const char *kTag = "RTOS_TIME_JIT";
        constexpr TickType_t kAllowedTaskJitterTicks = 2;
        constexpr std::int64_t kAllowedIrqWindowJitterUs = 2000;

        const TickType_t period = pdMS_TO_TICKS(10) > 0 ? pdMS_TO_TICKS(10) : 1;
        TickType_t wake = xTaskGetTickCount();
        TickType_t previous = wake;
        TickType_t minDelta = 0xFFFFFFFFu;
        TickType_t maxDelta = 0u;

        for (int i = 0; i < 30; ++i)
        {
            vTaskDelayUntil(&wake, period);
            const TickType_t now = xTaskGetTickCount();
            const TickType_t delta = now - previous;

            if (delta < minDelta)
            {
                minDelta = delta;
            }
            if (delta > maxDelta)
            {
                maxDelta = delta;
            }

            previous = now;
        }

        static portMUX_TYPE jitterMux = portMUX_INITIALIZER_UNLOCKED;
        std::int64_t minIrqWindowUs = 0x7FFFFFFFFFFFFFFFLL;
        std::int64_t maxIrqWindowUs = 0;

        for (int i = 0; i < 20; ++i)
        {
            const std::int64_t start = esp_timer_get_time();
            taskENTER_CRITICAL(&jitterMux);
            for (volatile std::uint32_t spin = 0u; spin < 300u; ++spin)
            {
            }
            taskEXIT_CRITICAL(&jitterMux);
            const std::int64_t elapsed = esp_timer_get_time() - start;

            if (elapsed < minIrqWindowUs)
            {
                minIrqWindowUs = elapsed;
            }
            if (elapsed > maxIrqWindowUs)
            {
                maxIrqWindowUs = elapsed;
            }

            vTaskDelay(1);
        }

        const TickType_t taskJitter = maxDelta - minDelta;
        const std::int64_t irqJitter = maxIrqWindowUs - minIrqWindowUs;

        if (taskJitter > kAllowedTaskJitterTicks)
        {
            Log::sys_error(kTag, "Task wake-up jitter too high: " + String(taskJitter) + " ticks");
            return false;
        }

        if (irqJitter > kAllowedIrqWindowJitterUs)
        {
            Log::sys_error(kTag, "ISR latency jitter proxy too high: " + String(static_cast<long long>(irqJitter)) + " us");
            return false;
        }

        Log::sys_info(kTag, "Jitter measurement successful");
        return true;
    }
} // namespace UnitTest
