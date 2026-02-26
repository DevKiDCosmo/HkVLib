#include "./random_task_creation_patterns.h"

#include <cstddef>
#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        std::uint32_t nextSeed(std::uint32_t seed)
        {
            return (seed * 1664525u) + 1013904223u;
        }

        void fuzzWorker(void *param)
        {
            auto *parent = static_cast<TaskHandle_t *>(param);
            taskYIELD();
            xTaskNotifyGive(*parent);
            vTaskDelete(nullptr);
        }
    } // namespace

    bool runRtosRandomTaskCreationPatternsFuzzTest()
    {
        constexpr const char *kTag = "RTOS_FUZZ_TASK";
        constexpr int kRounds = 20;

        TaskHandle_t parent = xTaskGetCurrentTaskHandle();
        std::uint32_t seed = 0xC0FFEE01u;

        for (int round = 0; round < kRounds; ++round)
        {
            seed = nextSeed(seed);
            const std::size_t toCreate = 1u + static_cast<std::size_t>(seed % 6u);

            std::size_t created = 0u;
            for (; created < toCreate; ++created)
            {
                if (xTaskCreate(fuzzWorker, "rt_fz_t", 2048, &parent, tskIDLE_PRIORITY + 1, nullptr) != pdPASS)
                {
                    break;
                }
            }

            std::size_t completed = 0u;
            while (completed < created)
            {
                const std::uint32_t signaled = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1500));
                if (signaled == 0u)
                {
                    Log::sys_error(kTag, "Completion timeout at round " + String(round));
                    return false;
                }

                completed += static_cast<std::size_t>(signaled);
                if (completed > created)
                {
                    completed = created;
                }
            }
        }

        Log::sys_info(kTag, "Random task creation patterns successful");
        return true;
    }
} // namespace UnitTest
