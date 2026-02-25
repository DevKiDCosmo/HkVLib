#include "./wake_source.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosWakeSourceTest()
    {
        constexpr const char *kTag = "RTOS_PWR_WAKE";
        const TickType_t before = xTaskGetTickCount();
        vTaskDelay(1);
        const TickType_t after = xTaskGetTickCount();

        if (after <= before)
        {
            Log::sys_error(kTag, "Wake-up source baseline failed");
            return false;
        }

        Log::sys_info(kTag, "Wake-up source baseline successful");
        return true;
    }
} // namespace UnitTest
