#include "./concurrency.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        struct CounterContext
        {
            SemaphoreHandle_t mutex;
            TaskHandle_t parent;
            volatile std::uint32_t counter;
        };

        void counterWorker(void *param)
        {
            auto *ctx = static_cast<CounterContext *>(param);
            for (std::uint32_t i = 0; i < 500u; ++i)
            {
                if (xSemaphoreTake(ctx->mutex, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    ctx->counter++;
                    xSemaphoreGive(ctx->mutex);
                }

                if ((i % 50u) == 0u)
                {
                    taskYIELD();
                }
            }

            xTaskNotifyGive(ctx->parent);
            vTaskDelete(nullptr);
        }
    } // namespace

    bool runRtosConcurrencyTest()
    {
        constexpr const char *kTag = "RTOS_CONC";

        CounterContext context{};
        context.mutex = xSemaphoreCreateMutex();
        context.parent = xTaskGetCurrentTaskHandle();
        context.counter = 0u;

        if (context.mutex == nullptr)
        {
            Log::sys_error(kTag, "Failed to create mutex");
            return false;
        }

        TaskHandle_t workerA = nullptr;
        TaskHandle_t workerB = nullptr;

        if (xTaskCreate(counterWorker, "rtos_conc_a", 3072, &context, tskIDLE_PRIORITY + 1, &workerA) != pdPASS ||
            xTaskCreate(counterWorker, "rtos_conc_b", 3072, &context, tskIDLE_PRIORITY + 1, &workerB) != pdPASS)
        {
            Log::sys_error(kTag, "Failed to create worker tasks");
            if (workerA != nullptr)
            {
                vTaskDelete(workerA);
            }
            if (workerB != nullptr)
            {
                vTaskDelete(workerB);
            }
            vSemaphoreDelete(context.mutex);
            return false;
        }

        const std::uint32_t expected = 1000u;
        for (int i = 0; i < 2; ++i)
        {
            if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(3000)) == 0)
            {
                Log::sys_error(kTag, "Worker completion timeout");
                vSemaphoreDelete(context.mutex);
                return false;
            }
        }

        vSemaphoreDelete(context.mutex);

        if (context.counter != expected)
        {
            Log::sys_error(kTag, "Counter mismatch: " + String(context.counter) + " != " + String(expected));
            return false;
        }

        Log::sys_info(kTag, "Concurrency correctness baseline successful");
        return true;
    }
} // namespace UnitTest
