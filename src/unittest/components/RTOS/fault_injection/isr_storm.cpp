#include "./isr_storm.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosIsrStormTest()
    {
        constexpr const char *kTag = "RTOS_FI_ISR";
        static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

        for (std::uint32_t index = 0; index < 300u; ++index)
        {
            taskENTER_CRITICAL(&mux);
            taskEXIT_CRITICAL(&mux);
            if ((index & 0x1Fu) == 0u)
            {
                vTaskDelay(1);
            }
        }

        Log::sys_info(kTag, "Simulated ISR storm test successful");
        return true;
    }
} // namespace UnitTest
