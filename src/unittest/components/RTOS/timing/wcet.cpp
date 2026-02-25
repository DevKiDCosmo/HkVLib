#include "./wcet.h"

#include <cstdint>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        struct LoadContext
        {
            volatile bool run;
            volatile std::uint32_t loops;
        };

        void loadTask(void *param)
        {
            auto *ctx = static_cast<LoadContext *>(param);
            while (ctx->run)
            {
                ctx->loops++;
                if ((ctx->loops & 0x1Fu) == 0u)
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

    bool runRtosWcetTest()
    {
        constexpr const char *kTag = "RTOS_TIME_WCET";
        constexpr std::int64_t kUpperBoundUs = 50000;

        LoadContext load{};
        load.run = true;

        if (xTaskCreate(loadTask, "rtos_time_load", 3072, &load, tskIDLE_PRIORITY + 1, nullptr) != pdPASS)
        {
            Log::sys_error(kTag, "Failed to create load task");
            return false;
        }

        std::int64_t wcetUs = 0;
        for (int sample = 0; sample < 30; ++sample)
        {
            const std::int64_t start = esp_timer_get_time();

            volatile std::uint32_t acc = 0u;
            for (std::uint32_t i = 0; i < 12000u; ++i)
            {
                acc += (i * 7u) ^ (i >> 1);
            }

            const std::int64_t elapsed = esp_timer_get_time() - start;
            if (elapsed > wcetUs)
            {
                wcetUs = elapsed;
            }

            (void)acc;
            vTaskDelay(1);
        }

        load.run = false;
        vTaskDelay(2);

        if (wcetUs <= 0 || wcetUs > kUpperBoundUs)
        {
            Log::sys_error(kTag, "WCET out of bounds: " + String(static_cast<long long>(wcetUs)) + " us");
            return false;
        }

        Log::sys_info(kTag, "WCET successful, upper bound=" + String(static_cast<long long>(wcetUs)) + " us");
        return true;
    }
} // namespace UnitTest
