#include "./shared_memory.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        struct SharedContext
        {
            SemaphoreHandle_t mutex;
            TaskHandle_t parent;
            volatile std::uint32_t value;
            volatile std::uint32_t done;
        };

        void sharedWorker(void *param)
        {
            auto *context = static_cast<SharedContext *>(param);
            for (std::uint32_t iteration = 0; iteration < 400u; ++iteration)
            {
                if (xSemaphoreTake(context->mutex, pdMS_TO_TICKS(50)) == pdTRUE)
                {
                    context->value += 1u;
                    xSemaphoreGive(context->mutex);
                }
                taskYIELD();
            }

            context->done += 1u;
            xTaskNotifyGive(context->parent);
            vTaskDelete(nullptr);
        }
    }

    bool runRtosSharedMemoryTest()
    {
        constexpr const char *kTag = "RTOS_IPC_SHM";

        SharedContext context{};
        context.mutex = xSemaphoreCreateMutex();
        context.parent = xTaskGetCurrentTaskHandle();

        if (context.mutex == nullptr)
        {
            Log::sys_error(kTag, "Failed to create mutex");
            return false;
        }

        if (xTaskCreate(sharedWorker, "ipc_shm_a", 3072, &context, tskIDLE_PRIORITY + 1, nullptr) != pdPASS ||
            xTaskCreate(sharedWorker, "ipc_shm_b", 3072, &context, tskIDLE_PRIORITY + 1, nullptr) != pdPASS)
        {
            vSemaphoreDelete(context.mutex);
            Log::sys_error(kTag, "Failed to create shared memory workers");
            return false;
        }

        bool ok = true;
        for (int idx = 0; idx < 2; ++idx)
        {
            if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1500)) == 0)
            {
                ok = false;
            }
        }

        vSemaphoreDelete(context.mutex);

        if (!ok || context.done != 2u || context.value != 800u)
        {
            Log::sys_error(kTag, "Shared memory race/data consistency failed");
            return false;
        }

        Log::sys_info(kTag, "Shared memory test successful");
        return true;
    }
} // namespace UnitTest
