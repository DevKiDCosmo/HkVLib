#include "./isr_to_task_signaling.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosIsrToTaskSignalingTest()
    {
        constexpr const char *kTag = "RTOS_INT_SIG";

        SemaphoreHandle_t sem = xSemaphoreCreateBinary();
        QueueHandle_t queue = xQueueCreate(4, sizeof(std::uint32_t));
        if (sem == nullptr || queue == nullptr)
        {
            if (sem != nullptr)
            {
                vSemaphoreDelete(sem);
            }
            if (queue != nullptr)
            {
                vQueueDelete(queue);
            }
            Log::sys_error(kTag, "Failed to create semaphore/queue");
            return false;
        }

        BaseType_t hpw = pdFALSE;
        const std::uint32_t payload = 0xA55AA55Au;

        if (xSemaphoreGiveFromISR(sem, &hpw) != pdTRUE)
        {
            vSemaphoreDelete(sem);
            vQueueDelete(queue);
            Log::sys_error(kTag, "xSemaphoreGiveFromISR failed");
            return false;
        }

        if (xQueueSendFromISR(queue, &payload, &hpw) != pdTRUE)
        {
            vSemaphoreDelete(sem);
            vQueueDelete(queue);
            Log::sys_error(kTag, "xQueueSendFromISR failed");
            return false;
        }

        if (xSemaphoreTake(sem, pdMS_TO_TICKS(100)) != pdTRUE)
        {
            vSemaphoreDelete(sem);
            vQueueDelete(queue);
            Log::sys_error(kTag, "Semaphore signaling not received");
            return false;
        }

        std::uint32_t out = 0u;
        if (xQueueReceive(queue, &out, pdMS_TO_TICKS(100)) != pdTRUE || out != payload)
        {
            vSemaphoreDelete(sem);
            vQueueDelete(queue);
            Log::sys_error(kTag, "Queue signaling not received/corrupted");
            return false;
        }

        vSemaphoreDelete(sem);
        vQueueDelete(queue);
        Log::sys_info(kTag, "ISR-to-task signaling baseline successful");
        return true;
    }
} // namespace UnitTest
