#include "./queue.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosQueueTest()
    {
        constexpr const char *kTag = "RTOS_IPC_Q";
        QueueHandle_t queue = xQueueCreate(3, sizeof(std::uint32_t));
        if (queue == nullptr)
        {
            Log::sys_error(kTag, "Failed to create queue");
            return false;
        }

        const std::uint32_t first = 1u;
        const std::uint32_t second = 2u;
        const std::uint32_t third = 3u;
        const std::uint32_t fourth = 4u;

        bool ok = true;
        ok = (xQueueSend(queue, &first, 0) == pdTRUE) && ok;
        ok = (xQueueSend(queue, &second, 0) == pdTRUE) && ok;
        ok = (xQueueSend(queue, &third, 0) == pdTRUE) && ok;

        if (xQueueSend(queue, &fourth, 0) == pdTRUE)
        {
            ok = false;
            Log::sys_error(kTag, "Queue overflow handling failed");
        }

        std::uint32_t out = 0u;
        ok = (xQueueReceive(queue, &out, pdMS_TO_TICKS(50)) == pdTRUE && out == first) && ok;
        ok = (xQueueReceive(queue, &out, pdMS_TO_TICKS(50)) == pdTRUE && out == second) && ok;
        ok = (xQueueReceive(queue, &out, pdMS_TO_TICKS(50)) == pdTRUE && out == third) && ok;

        if (xQueueReceive(queue, &out, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            ok = false;
            Log::sys_error(kTag, "Queue blocking semantics failed");
        }

        vQueueDelete(queue);

        if (!ok)
        {
            return false;
        }

        Log::sys_info(kTag, "Queue test successful");
        return true;
    }
} // namespace UnitTest
