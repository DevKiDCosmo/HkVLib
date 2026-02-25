#include "./condition_variables.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        struct NotifyContext
        {
            TaskHandle_t waiter;
            TaskHandle_t parent;
        };

        void waiterTask(void *param)
        {
            auto *ctx = static_cast<NotifyContext *>(param);
            ctx->waiter = xTaskGetCurrentTaskHandle();
            xTaskNotifyGive(ctx->parent);

            if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000)) > 0)
            {
                xTaskNotifyGive(ctx->parent);
            }

            vTaskDelete(nullptr);
        }
    } // namespace

    bool runRtosConditionVariableTest()
    {
        constexpr const char *kTag = "RTOS_SYNC_CV";

        NotifyContext context{};
        context.parent = xTaskGetCurrentTaskHandle();

        if (xTaskCreate(waiterTask, "rtos_cv_waiter", 3072, &context, tskIDLE_PRIORITY + 1, nullptr) != pdPASS)
        {
            Log::sys_error(kTag, "Failed to create waiter task");
            return false;
        }

        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000)) == 0)
        {
            Log::sys_error(kTag, "Waiter did not become ready");
            return false;
        }

        if (context.waiter == nullptr)
        {
            Log::sys_error(kTag, "Waiter handle missing");
            return false;
        }

        xTaskNotifyGive(context.waiter);
        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000)) == 0)
        {
            Log::sys_error(kTag, "Waiter was not signaled");
            return false;
        }

        Log::sys_info(kTag, "Condition-variable equivalent baseline successful");
        return true;
    }
} // namespace UnitTest
