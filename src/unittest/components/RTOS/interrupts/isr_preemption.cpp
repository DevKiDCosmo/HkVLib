#include "./isr_preemption.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosIsrPreemptionTest()
    {
        constexpr const char *kTag = "RTOS_INT_PRE";
        static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

        const TickType_t before = xTaskGetTickCount();

        taskENTER_CRITICAL(&mux);
        taskENTER_CRITICAL(&mux);
        for (volatile std::uint32_t i = 0u; i < 500u; ++i)
        {
        }
        taskEXIT_CRITICAL(&mux);
        taskEXIT_CRITICAL(&mux);

        vTaskDelay(1);
        const TickType_t after = xTaskGetTickCount();
        if (after <= before)
        {
            Log::sys_error(kTag, "Tick did not progress after nested critical section");
            return false;
        }

        Log::sys_info(kTag, "ISR preemption/nesting baseline successful");
        return true;
    }
} // namespace UnitTest
