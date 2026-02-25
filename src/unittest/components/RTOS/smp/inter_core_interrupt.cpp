#include "./inter_core_interrupt.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        struct InterCoreContext
        {
            TaskHandle_t parent;
            volatile int workerCore;
        };

        void interCoreWorker(void *param)
        {
            auto *context = static_cast<InterCoreContext *>(param);
            context->workerCore = xPortGetCoreID();
            xTaskNotifyGive(context->parent);
            vTaskDelete(nullptr);
        }
    }

    bool runRtosInterCoreInterruptTest()
    {
        constexpr const char *kTag = "RTOS_SMP_ICI";
        InterCoreContext context{};
        context.parent = xTaskGetCurrentTaskHandle();
        context.workerCore = -1;

        const BaseType_t workerCore = xPortGetCoreID() == 0 ? 1 : 0;
        if (xTaskCreatePinnedToCore(interCoreWorker, "smp_ici", 3072, &context, tskIDLE_PRIORITY + 1, nullptr, workerCore) != pdPASS)
        {
            Log::sys_error(kTag, "Failed to create inter-core worker");
            return false;
        }

        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(700)) == 0)
        {
            Log::sys_error(kTag, "Inter-core signaling timeout");
            return false;
        }

        Log::sys_info(kTag, "Inter-core interrupt proxy successful");
        return true;
    }
} // namespace UnitTest
