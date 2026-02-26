#include "./tasks_100_plus.h"

#include <cstddef>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        void stressWorker(void *param)
        {
            auto *parent = static_cast<TaskHandle_t *>(param);
            vTaskDelay(1);
            xTaskNotifyGive(*parent);
            vTaskDelete(nullptr);
        }
    } // namespace

    bool runRtos100PlusTasksStressTest()
    {
        constexpr const char *kTag = "RTOS_STR_100";
        constexpr std::size_t kTargetTasks = 100u;

        TaskHandle_t parent = xTaskGetCurrentTaskHandle();
        std::size_t created = 0u;

        for (; created < kTargetTasks; ++created)
        {
            if (xTaskCreate(stressWorker, "rtos_s100", 2048, &parent, tskIDLE_PRIORITY + 1, nullptr) != pdPASS)
            {
                break;
            }
        }

        if (created < 16u)
        {
            Log::sys_error(kTag, "Too few stress tasks created: " + String(created));
            return false;
        }

        std::size_t completed = 0u;
        while (completed < created)
        {
            const std::uint32_t signaled = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(3000));
            if (signaled == 0u)
            {
                Log::sys_error(kTag, "Task completion timeout, completed=" + String(completed) + ", created=" + String(created));
                return false;
            }

            completed += static_cast<std::size_t>(signaled);
            if (completed > created)
            {
                completed = created;
            }
        }

        Log::sys_info(kTag, "100+ tasks stress baseline successful, created=" + String(created));
        return true;
    }
} // namespace UnitTest
