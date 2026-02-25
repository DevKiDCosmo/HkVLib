#include "./semaphore.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosSemaphoreTest()
    {
        constexpr const char *kTag = "RTOS_SYNC_SEM";

        SemaphoreHandle_t binary = xSemaphoreCreateBinary();
        SemaphoreHandle_t counting = xSemaphoreCreateCounting(3, 0);
        if (binary == nullptr || counting == nullptr)
        {
            if (binary != nullptr)
            {
                vSemaphoreDelete(binary);
            }
            if (counting != nullptr)
            {
                vSemaphoreDelete(counting);
            }
            Log::sys_error(kTag, "Failed to create semaphores");
            return false;
        }

        BaseType_t hpw = pdFALSE;
        xSemaphoreGiveFromISR(binary, &hpw);
        if (xSemaphoreTake(binary, pdMS_TO_TICKS(100)) != pdTRUE)
        {
            vSemaphoreDelete(binary);
            vSemaphoreDelete(counting);
            Log::sys_error(kTag, "Binary semaphore behavior failed");
            return false;
        }

        xSemaphoreGive(counting);
        xSemaphoreGive(counting);
        if (uxSemaphoreGetCount(counting) != 2u)
        {
            vSemaphoreDelete(binary);
            vSemaphoreDelete(counting);
            Log::sys_error(kTag, "Counting semaphore limits failed");
            return false;
        }

        vSemaphoreDelete(binary);
        vSemaphoreDelete(counting);
        Log::sys_info(kTag, "Semaphore baseline successful");
        return true;
    }
} // namespace UnitTest
