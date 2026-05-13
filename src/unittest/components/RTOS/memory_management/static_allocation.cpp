#include "./static_allocation.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        constexpr UBaseType_t kStaticStackDepthWords = 2048u;
        constexpr std::uint32_t kExpectedComputation = 99990000u;

        struct StaticAllocationContext
        {
            TaskHandle_t parent;
            volatile bool started;
            volatile bool completed;
            volatile std::uint32_t computation;
        };

        static StaticTask_t gStaticTaskBuffer;
        static StackType_t gStaticStack[kStaticStackDepthWords];

        void staticAllocationWorker(void *param)
        {
            auto *context = static_cast<StaticAllocationContext *>(param);
            context->started = true;

            std::uint32_t acc = 0u;
            for (std::uint32_t i = 0u; i < 10000u; ++i)
            {
                acc += (i * 2u);
            }

            context->computation = acc;
            context->completed = true;
            xTaskNotifyGive(context->parent);
            vTaskDelete(nullptr);
        }
    } // namespace

    bool runRtosStaticAllocationTest()
    {
        constexpr const char *kTag = "RTOS_MEM_STA";
        StaticAllocationContext context{};
        context.parent = xTaskGetCurrentTaskHandle();

        TaskHandle_t task = xTaskCreateStatic(
            staticAllocationWorker,
            "rtos_static_alloc",
            kStaticStackDepthWords,
            &context,
            tskIDLE_PRIORITY + 1,
            gStaticStack,
            &gStaticTaskBuffer);

        if (task == nullptr)
        {
            Log::sys_error(kTag, "xTaskCreateStatic failed");
            return false;
        }

        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(2000)) == 0u)
        {
            vTaskDelete(task);
            Log::sys_error(kTag, "Static worker did not complete in time");
            return false;
        }

        if (!context.started || !context.completed)
        {
            Log::sys_error(kTag, "Static worker handshake flags invalid");
            return false;
        }

        if (context.computation != kExpectedComputation)
        {
            Log::sys_error(
                kTag,
                "Static worker computation mismatch: actual=" + String(context.computation) +
                    ", expected=" + String(kExpectedComputation));
            return false;
        }

        Log::sys_info(kTag, "Static allocation xTaskCreateStatic path successful");
        return true;
    }
} // namespace UnitTest
