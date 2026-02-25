#include "./round_robin.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        struct RoundRobinContext
        {
            volatile bool run;
            volatile std::uint32_t counterA;
            volatile std::uint32_t counterB;
        };

        void workerA(void *param)
        {
            auto *ctx = static_cast<RoundRobinContext *>(param);
            while (ctx->run)
            {
                ctx->counterA++;
                if ((ctx->counterA & 0x1Fu) == 0u)
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

        void workerB(void *param)
        {
            auto *ctx = static_cast<RoundRobinContext *>(param);
            while (ctx->run)
            {
                ctx->counterB++;
                if ((ctx->counterB & 0x1Fu) == 0u)
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

    bool runRtosRoundRobinTest()
    {
        constexpr const char *kTag = "RTOS_SCH_RR";
        RoundRobinContext context{};
        context.run = true;

        if (xTaskCreate(workerA, "rtos_rr_a", 3072, &context, tskIDLE_PRIORITY + 2, nullptr) != pdPASS ||
            xTaskCreate(workerB, "rtos_rr_b", 3072, &context, tskIDLE_PRIORITY + 2, nullptr) != pdPASS)
        {
            Log::sys_error(kTag, "Failed to create round-robin workers");
            context.run = false;
            vTaskDelay(1);
            return false;
        }

        vTaskDelay(20);
        context.run = false;
        vTaskDelay(2);

        if (context.counterA == 0u || context.counterB == 0u)
        {
            Log::sys_error(kTag, "Round-robin counters not progressing");
            return false;
        }

        Log::sys_info(kTag, "Round-robin baseline successful");
        return true;
    }
} // namespace UnitTest
