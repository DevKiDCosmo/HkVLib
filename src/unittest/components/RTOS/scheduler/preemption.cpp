#include "./preemption.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        struct PreemptionContext
        {
            TaskHandle_t parent;
            volatile bool highRan;
        };

        void highReadyTask(void *param)
        {
            auto *ctx = static_cast<PreemptionContext *>(param);
            ctx->highRan = true;
            xTaskNotifyGive(ctx->parent);
            vTaskDelete(nullptr);
        }
    } // namespace

    bool runRtosPreemptionTest()
    {
        constexpr const char *kTag = "RTOS_SCH_PRE";

        PreemptionContext context{};
        context.parent = xTaskGetCurrentTaskHandle();
        context.highRan = false;

        const TickType_t beforeCreate = xTaskGetTickCount();
        if (xTaskCreate(highReadyTask, "rtos_preempt_high", 3072, &context, tskIDLE_PRIORITY + 3, nullptr) != pdPASS)
        {
            Log::sys_error(kTag, "Failed to create high-priority task");
            return false;
        }

        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000)) == 0)
        {
            Log::sys_error(kTag, "High-priority task did not run in time");
            return false;
        }

        const TickType_t elapsed = xTaskGetTickCount() - beforeCreate;
        if (!context.highRan || elapsed > pdMS_TO_TICKS(50))
        {
            Log::sys_error(kTag, "Preemption baseline failed");
            return false;
        }

        Log::sys_info(kTag, "Preemption baseline successful");
        return true;
    }
} // namespace UnitTest
