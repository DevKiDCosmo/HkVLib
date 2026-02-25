#include "./artificial_delay.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosArtificialDelayInjectionTest()
    {
        constexpr const char *kTag = "RTOS_FI_DLY";
        const TickType_t before = xTaskGetTickCount();
        vTaskDelay(pdMS_TO_TICKS(120));
        const TickType_t after = xTaskGetTickCount();

        if (after <= before)
        {
            Log::sys_error(kTag, "Artificial delay injection test failed");
            return false;
        }

        Log::sys_info(kTag, "Artificial delay injection successful");
        return true;
    }
} // namespace UnitTest
