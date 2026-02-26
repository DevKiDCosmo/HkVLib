#include "./systick_configuration.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosPortSysTickConfigurationTest()
    {
        constexpr const char *kTag = "RTOS_PORT_TICK";

        if (configTICK_RATE_HZ <= 0)
        {
            Log::sys_error(kTag, "Invalid tick rate configuration");
            return false;
        }

        const TickType_t before = xTaskGetTickCount();
        vTaskDelay(1);
        const TickType_t after = xTaskGetTickCount();
        if (after <= before)
        {
            Log::sys_error(kTag, "SysTick did not advance");
            return false;
        }

        Log::sys_info(kTag, "SysTick configuration baseline successful");
        return true;
    }
} // namespace UnitTest
