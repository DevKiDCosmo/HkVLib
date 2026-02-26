#include "./interrupt_behavior.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosInterruptBehaviorTest()
    {
        constexpr const char *kTag = "RTOS_IRQ";
        static portMUX_TYPE criticalMux = portMUX_INITIALIZER_UNLOCKED;

        const TickType_t before = xTaskGetTickCount();
        taskENTER_CRITICAL(&criticalMux);
        taskEXIT_CRITICAL(&criticalMux);

        vTaskDelay(1);
        const TickType_t after = xTaskGetTickCount();
        if (after <= before)
        {
            Log::sys_error(kTag, "Tick did not progress after critical section");
            return false;
        }

        Log::sys_info(kTag, "Interrupt behavior baseline successful");
        return true;
    }
} // namespace UnitTest
