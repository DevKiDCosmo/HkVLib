#include "./rapid_create_delete.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        void rapidWorker(void *param)
        {
            auto *parent = static_cast<TaskHandle_t *>(param);
            xTaskNotifyGive(*parent);
            vTaskDelete(nullptr);
        }
    } // namespace

    bool runRtosRapidCreateDeleteStressTest()
    {
        constexpr const char *kTag = "RTOS_STR_RCD";
        constexpr int kIterations = 40;

        TaskHandle_t parent = xTaskGetCurrentTaskHandle();

        for (int i = 0; i < kIterations; ++i)
        {
            TaskHandle_t worker = nullptr;
            if (xTaskCreate(rapidWorker, "rtos_rcd", 2048, &parent, tskIDLE_PRIORITY + 1, &worker) != pdPASS)
            {
                Log::sys_error(kTag, "Failed to create task at iteration " + String(i));
                return false;
            }

            if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1500)) == 0)
            {
                Log::sys_error(kTag, "Timeout waiting task at iteration " + String(i));
                if (worker != nullptr)
                {
                    vTaskDelete(worker);
                }
                return false;
            }
        }

        Log::sys_info(kTag, "Rapid create/delete stress successful");
        return true;
    }
} // namespace UnitTest
