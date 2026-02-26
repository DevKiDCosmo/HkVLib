#include "./tickless_idle.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosTicklessIdleTest()
    {
        constexpr const char *kTag = "RTOS_PWR_IDLE";
        const TickType_t start = xTaskGetTickCount();
        vTaskDelay(pdMS_TO_TICKS(100));
        const TickType_t end = xTaskGetTickCount();

        if (end <= start)
        {
            Log::sys_error(kTag, "Tickless/idle progression failed");
            return false;
        }

        int deltaMs = static_cast<int>(end - start) * portTICK_PERIOD_MS;
        if (deltaMs < 80 || deltaMs > 120)
        {
            Log::sys_error(kTag, "Tickless/idle delta out of expected range: " + String(deltaMs) + "ms");
            return false;
        }

        Log::sys_info(kTag, "Tickless idle delta: " + String(deltaMs) + "ms");
        Log::sys_info(kTag, "Tickless idle baseline successful");
        return true;
    }
} // namespace UnitTest
