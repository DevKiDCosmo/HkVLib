#include "./mutex.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosMutexTest()
    {
        constexpr const char *kTag = "RTOS_SYNC_MTX";

        SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
        if (mutex == nullptr)
        {
            Log::sys_error(kTag, "Failed to create mutex");
            return false;
        }

        std::uint32_t value = 0u;
        if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) != pdTRUE)
        {
            vSemaphoreDelete(mutex);
            Log::sys_error(kTag, "Mutex take failed");
            return false;
        }

        value++;
        if (xSemaphoreGive(mutex) != pdTRUE)
        {
            vSemaphoreDelete(mutex);
            Log::sys_error(kTag, "Mutex give failed");
            return false;
        }

        vSemaphoreDelete(mutex);
        if (value != 1u)
        {
            Log::sys_error(kTag, "Mutex value mismatch");
            return false;
        }

        Log::sys_info(kTag, "Mutex baseline successful");
        return true;
    }
} // namespace UnitTest
