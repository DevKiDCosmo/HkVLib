#include "./interrupt_latency.h"

#include <cstdint>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosInterruptLatencyTest()
    {
        constexpr const char *kTag = "RTOS_INT_LAT";
        constexpr std::int64_t kMaxLatencyUs = 3000;
        static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

        std::int64_t maxWindowUs = 0;
        for (int sample = 0; sample < 25; ++sample)
        {
            const std::int64_t start = esp_timer_get_time();
            taskENTER_CRITICAL(&mux);
            for (volatile std::uint32_t spin = 0u; spin < 300u; ++spin)
            {
            }
            taskEXIT_CRITICAL(&mux);
            const std::int64_t elapsed = esp_timer_get_time() - start;

            if (elapsed > maxWindowUs)
            {
                maxWindowUs = elapsed;
            }

            vTaskDelay(1);
        }

        if (maxWindowUs > kMaxLatencyUs)
        {
            Log::sys_error(kTag, "Interrupt latency proxy too high: " + String(static_cast<long long>(maxWindowUs)) + " us");
            return false;
        }

        Log::sys_info(kTag, "Interrupt latency baseline successful");
        return true;
    }
} // namespace UnitTest
