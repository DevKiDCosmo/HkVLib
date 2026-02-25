#include "./context_switch_integrity.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        struct WorkerParams
        {
            TaskHandle_t parent;
            volatile std::uint32_t *resultSlot;
            volatile std::uint32_t *doneCount;
        };

        struct IntegrityContext
        {
            volatile std::uint32_t doneCount;
            volatile std::uint32_t checksumA;
            volatile std::uint32_t checksumB;
            WorkerParams workerA;
            WorkerParams workerB;
        };

        void integrityWorker(void *param)
        {
            auto *params = static_cast<WorkerParams *>(param);
            std::uint32_t acc = 0x12345678u;

            for (std::uint32_t i = 0; i < 10000u; ++i)
            {
                acc = (acc << 5) ^ (acc >> 2) ^ i;
                if ((i & 0x3Fu) == 0u)
                {
                    vTaskDelay(1);
                }
                else
                {
                    taskYIELD();
                }
            }

            *params->resultSlot = acc;
            (*params->doneCount)++;
            xTaskNotifyGive(params->parent);
            vTaskDelete(nullptr);
        }
    } // namespace

    bool runRtosContextSwitchIntegrityTest()
    {
        constexpr const char *kTag = "RTOS_SCH_CTX";

        IntegrityContext context{};
        context.workerA.parent = xTaskGetCurrentTaskHandle();
        context.workerA.resultSlot = &context.checksumA;
        context.workerA.doneCount = &context.doneCount;

        context.workerB.parent = context.workerA.parent;
        context.workerB.resultSlot = &context.checksumB;
        context.workerB.doneCount = &context.doneCount;

        if (xTaskCreate(integrityWorker, "rtos_ctx_a", 3072, &context.workerA, tskIDLE_PRIORITY + 2, nullptr) != pdPASS ||
            xTaskCreate(integrityWorker, "rtos_ctx_b", 3072, &context.workerB, tskIDLE_PRIORITY + 2, nullptr) != pdPASS)
        {
            Log::sys_error(kTag, "Failed to create context integrity workers");
            return false;
        }

        std::uint32_t received = 0u;
        while (received < 2u)
        {
            const std::uint32_t value = ulTaskNotifyTake(pdFALSE, pdMS_TO_TICKS(5000));
            if (value == 0u)
            {
                Log::sys_error(kTag, "Context integrity worker timeout");
                return false;
            }

            received++;
        }

        if (context.doneCount != 2u || context.checksumA == 0u || context.checksumB == 0u)
        {
            Log::sys_error(kTag, "Context switch integrity baseline failed");
            return false;
        }

        Log::sys_info(kTag, "Context switch integrity baseline successful");
        return true;
    }
} // namespace UnitTest
