#include "./priority_inversion.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        struct PriorityInversionContext
        {
            SemaphoreHandle_t mutex;
            TaskHandle_t parent;
            volatile bool highCompleted;
        };

        void highTask(void *param)
        {
            auto *ctx = static_cast<PriorityInversionContext *>(param);
            if (xSemaphoreTake(ctx->mutex, pdMS_TO_TICKS(500)) == pdTRUE)
            {
                ctx->highCompleted = true;
                xSemaphoreGive(ctx->mutex);
            }
            xTaskNotifyGive(ctx->parent);
            vTaskDelete(nullptr);
        }
    } // namespace

    bool runRtosPriorityInversionTest()
    {
        constexpr const char *kTag = "RTOS_DL_PRIO";

        PriorityInversionContext context{};
        context.mutex = xSemaphoreCreateMutex();
        context.parent = xTaskGetCurrentTaskHandle();

        if (context.mutex == nullptr)
        {
            Log::sys_error(kTag, "Failed to create mutex");
            return false;
        }

        if (xSemaphoreTake(context.mutex, pdMS_TO_TICKS(100)) != pdTRUE)
        {
            vSemaphoreDelete(context.mutex);
            Log::sys_error(kTag, "Failed to lock mutex in low-priority phase");
            return false;
        }

        if (xTaskCreate(highTask, "rtos_dl_high", 3072, &context, tskIDLE_PRIORITY + 3, nullptr) != pdPASS)
        {
            xSemaphoreGive(context.mutex);
            vSemaphoreDelete(context.mutex);
            Log::sys_error(kTag, "Failed to create high-priority waiter task");
            return false;
        }

        vTaskDelay(5);
        xSemaphoreGive(context.mutex);

        const bool notified = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000)) > 0;
        vSemaphoreDelete(context.mutex);

        if (!notified || !context.highCompleted)
        {
            Log::sys_error(kTag, "Priority inversion baseline failed");
            return false;
        }

        Log::sys_info(kTag, "Priority inversion baseline successful");
        return true;
    }
} // namespace UnitTest
