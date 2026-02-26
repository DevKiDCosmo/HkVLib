#include "./maximum_queue_usage.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosMaximumQueueUsageStressTest()
    {
        constexpr const char *kTag = "RTOS_STR_QUE";
        constexpr UBaseType_t kQueueLength = 32u;

        QueueHandle_t queue = xQueueCreate(kQueueLength, sizeof(std::uint32_t));
        if (queue == nullptr)
        {
            Log::sys_error(kTag, "Failed to create queue");
            return false;
        }

        std::uint32_t value = 0u;
        for (; value < kQueueLength; ++value)
        {
            if (xQueueSend(queue, &value, 0) != pdTRUE)
            {
                Log::sys_error(kTag, "Queue fill failed at index " + String(value));
                vQueueDelete(queue);
                return false;
            }
        }

        const std::uint32_t overflowValue = 0xDEADBEEFu;
        if (xQueueSend(queue, &overflowValue, 0) == pdTRUE)
        {
            Log::sys_error(kTag, "Queue accepted data beyond max usage");
            vQueueDelete(queue);
            return false;
        }

        vQueueDelete(queue);
        Log::sys_info(kTag, "Maximum queue usage stress successful");
        return true;
    }
} // namespace UnitTest
