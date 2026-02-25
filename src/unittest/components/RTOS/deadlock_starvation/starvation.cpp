#include "./starvation.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        struct StarvationContext
        {
            volatile bool run;
            volatile std::uint32_t low;
            volatile std::uint32_t mid;
        };

        void lowTask(void *param)
        {
            auto *ctx = static_cast<StarvationContext *>(param);
            while (ctx->run)
            {
                ctx->low++;
                vTaskDelay(1);
            }
            vTaskDelete(nullptr);
        }

        void midTask(void *param)
        {
            auto *ctx = static_cast<StarvationContext *>(param);
            while (ctx->run)
            {
                ctx->mid++;
                vTaskDelay(1);
            }
            vTaskDelete(nullptr);
        }
    } // namespace

    bool runRtosStarvationSimulationTest()
    {
        constexpr const char *kTag = "RTOS_DL_STARV";

        StarvationContext context{};
        context.run = true;

        if (xTaskCreate(lowTask, "rtos_dl_low", 3072, &context, tskIDLE_PRIORITY + 1, nullptr) != pdPASS ||
            xTaskCreate(midTask, "rtos_dl_mid", 3072, &context, tskIDLE_PRIORITY + 2, nullptr) != pdPASS)
        {
            context.run = false;
            Log::sys_error(kTag, "Failed to create starvation simulation tasks");
            return false;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
        context.run = false;
        vTaskDelay(2);

        if (context.low == 0u || context.mid == 0u)
        {
            Log::sys_error(kTag, "Starvation simulation failed");
            return false;
        }

        Log::sys_info(kTag, "Starvation simulation baseline successful");
        return true;
    }
} // namespace UnitTest
