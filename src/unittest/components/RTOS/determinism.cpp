#include "./determinism.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../serial/log.h"

namespace UnitTest
{
    bool runRtosDeterminismTest()
    {
        constexpr const char *kTag = "RTOS_DET";

        TickType_t previous = xTaskGetTickCount();
        for (int i = 0; i < 6; ++i)
        {
            vTaskDelay(1);
            const TickType_t current = xTaskGetTickCount();
            const TickType_t delta = current - previous;

            if (delta < 1 || delta > 2)
            {
                Log::sys_error(kTag, "Unexpected tick delta at step " + String(i) + ": " + String(delta));
                return false;
            }

            previous = current;
        }

        Log::sys_info(kTag, "Determinism baseline successful");
        return true;
    }
} // namespace UnitTest
