#include "./cross_core_sync.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        struct CrossCoreContext
        {
            SemaphoreHandle_t mutex;
            TaskHandle_t parent;
            volatile std::uint32_t counter;
        };

        void crossCoreWorker(void *param)
        {
            auto *context = static_cast<CrossCoreContext *>(param);
            for (std::uint32_t iteration = 0; iteration < 200u; ++iteration)
            {
                if (xSemaphoreTake(context->mutex, pdMS_TO_TICKS(50)) == pdTRUE)
                {
                    context->counter += 1u;
                    xSemaphoreGive(context->mutex);
                }
                vTaskDelay(1);
            }

            xTaskNotifyGive(context->parent);
            vTaskDelete(nullptr);
        }
    }

    bool runRtosCrossCoreSynchronizationTest()
    {
        constexpr const char *kTag = "RTOS_SMP_SYNC";

        CrossCoreContext context{};
        context.mutex = xSemaphoreCreateMutex();
        context.parent = xTaskGetCurrentTaskHandle();

        if (context.mutex == nullptr)
        {
            Log::sys_error(kTag, "Failed to create cross-core mutex");
            return false;
        }

        const BaseType_t currentCore = xPortGetCoreID();
        const BaseType_t otherCore = currentCore == 0 ? 1 : 0;

        bool created = true;
        created = (xTaskCreatePinnedToCore(crossCoreWorker, "smp_sync_a", 3072, &context, tskIDLE_PRIORITY + 1, nullptr, currentCore) == pdPASS) && created;
        created = (xTaskCreatePinnedToCore(crossCoreWorker, "smp_sync_b", 3072, &context, tskIDLE_PRIORITY + 1, nullptr, otherCore) == pdPASS) && created;

        if (!created)
        {
            vSemaphoreDelete(context.mutex);
            Log::sys_error(kTag, "Failed to create cross-core sync workers");
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

        if (!ok || context.counter != 400u)
        {
            Log::sys_error(kTag, "Cross-core synchronization failed");
            return false;
        }

        Log::sys_info(kTag, "Cross-core synchronization successful");
        return true;
    }
} // namespace UnitTest
