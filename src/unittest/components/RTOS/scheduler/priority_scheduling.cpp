#include "./priority_scheduling.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        struct PriorityContext
        {
            volatile std::uint32_t lowCounter;
            volatile std::uint32_t highCounter;
            volatile bool run;
        };

        struct PriorityProbeContext
        {
            TaskHandle_t parent;
            volatile UBaseType_t observedPriority;
        };

        void priorityProbeTask(void *param)
        {
            auto *ctx = static_cast<PriorityProbeContext *>(param);
            ctx->observedPriority = uxTaskPriorityGet(nullptr);
            xTaskNotifyGive(ctx->parent);
            vTaskDelete(nullptr);
        }

        void lowTask(void *param)
        {
            auto *ctx = static_cast<PriorityContext *>(param);
            while (ctx->run)
            {
                ctx->lowCounter++;
                if ((ctx->lowCounter & 0x1Fu) == 0u)
                {
                    vTaskDelay(1);
                }
                else
                {
                    taskYIELD();
                }
            }
            vTaskDelete(nullptr);
        }

        void highTask(void *param)
        {
            auto *ctx = static_cast<PriorityContext *>(param);
            for (int i = 0; i < 200; ++i)
            {
                ctx->highCounter++;
                if ((ctx->highCounter & 0x1Fu) == 0u)
                {
                    vTaskDelay(1);
                }
                else
                {
                    taskYIELD();
                }
            }
            vTaskDelete(nullptr);
        }
    } // namespace

    bool runRtosPrioritySchedulingTest()
    {
        constexpr const char *kTag = "RTOS_SCH_PRIO";

        PriorityProbeContext probe{};
        probe.parent = xTaskGetCurrentTaskHandle();

        const UBaseType_t invalidPriority = configMAX_PRIORITIES + 1u;
        if (xTaskCreate(priorityProbeTask, "rtos_prio_probe", 2048, &probe, invalidPriority, nullptr) != pdPASS)
        {
            Log::sys_error(kTag, "Invalid-priority boundary probe task create failed");
            return false;
        }

        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000)) == 0)
        {
            Log::sys_error(kTag, "Invalid-priority boundary probe timeout");
            return false;
        }

        if (probe.observedPriority >= configMAX_PRIORITIES)
        {
            Log::sys_error(
                kTag,
                "Invalid-priority boundary handling failed: observed=" + String(probe.observedPriority) +
                    ", max=" + String(configMAX_PRIORITIES));
            return false;
        }

        PriorityContext context{};
        context.run = true;

        TaskHandle_t low = nullptr;
        TaskHandle_t high = nullptr;

        if (xTaskCreate(lowTask, "rtos_prio_low", 3072, &context, tskIDLE_PRIORITY + 1, &low) != pdPASS)
        {
            Log::sys_error(kTag, "Failed to create low-priority task");
            return false;
        }

        vTaskDelay(2);
        const std::uint32_t lowBeforeHigh = context.lowCounter;

        if (xTaskCreate(highTask, "rtos_prio_high", 3072, &context, tskIDLE_PRIORITY + 3, &high) != pdPASS)
        {
            Log::sys_error(kTag, "Failed to create high-priority task");
            context.run = false;
            vTaskDelay(1);
            return false;
        }

        vTaskDelay(10);
        context.run = false;
        vTaskDelay(2);

        if (context.highCounter == 0u || context.lowCounter <= lowBeforeHigh)
        {
            Log::sys_error(kTag, "Priority scheduling baseline failed");
            return false;
        }

        Log::sys_info(kTag, "Priority scheduling baseline successful");
        return true;
    }
} // namespace UnitTest
