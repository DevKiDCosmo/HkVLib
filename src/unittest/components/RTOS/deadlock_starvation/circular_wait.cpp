#include "./circular_wait.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosCircularWaitTest()
    {
        constexpr const char *kTag = "RTOS_DL_CIRC";

        SemaphoreHandle_t a = xSemaphoreCreateMutex();
        SemaphoreHandle_t b = xSemaphoreCreateMutex();
        if (a == nullptr || b == nullptr)
        {
            if (a != nullptr)
            {
                vSemaphoreDelete(a);
            }
            if (b != nullptr)
            {
                vSemaphoreDelete(b);
            }
            Log::sys_error(kTag, "Failed to create mutexes");
            return false;
        }

        const bool first = xSemaphoreTake(a, pdMS_TO_TICKS(100)) == pdTRUE;
        const bool second = first && (xSemaphoreTake(b, pdMS_TO_TICKS(100)) == pdTRUE);

        if (second)
        {
            xSemaphoreGive(b);
            xSemaphoreGive(a);
        }

        vSemaphoreDelete(a);
        vSemaphoreDelete(b);

        if (!second)
        {
            Log::sys_error(kTag, "Circular wait mitigation failed");
            return false;
        }

        Log::sys_info(kTag, "Circular wait baseline successful");
        return true;
    }
} // namespace UnitTest
