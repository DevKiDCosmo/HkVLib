#include "./disable_enable_safety.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosDisableEnableInterruptSafetyTest()
    {
        constexpr const char *kTag = "RTOS_INT_SAFE";
        static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

        const TickType_t before = xTaskGetTickCount();
        taskENTER_CRITICAL(&mux);
        for (volatile std::uint32_t i = 0u; i < 1000u; ++i)
        {
        }
        taskEXIT_CRITICAL(&mux);
        vTaskDelay(1);
        const TickType_t after = xTaskGetTickCount();

        if (after <= before)
        {
            Log::sys_error(kTag, "No tick progress after critical section");
            return false;
        }

        Log::sys_info(kTag, "Disable/enable interrupt safety baseline successful");
        return true;
    }
} // namespace UnitTest
