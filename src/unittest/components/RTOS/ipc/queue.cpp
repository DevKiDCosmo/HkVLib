#include "./queue.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        BaseType_t safeQueueSend(QueueHandle_t queue, const void *item, TickType_t timeout)
        {
            if (queue == nullptr)
            {
                return pdFAIL;
            }

            return xQueueSend(queue, item, timeout);
        }

        BaseType_t safeQueueReceive(QueueHandle_t queue, void *out, TickType_t timeout)
        {
            if (queue == nullptr)
            {
                return pdFAIL;
            }

            return xQueueReceive(queue, out, timeout);
        }
    } // namespace

    bool runRtosQueueTest()
    {
        constexpr const char *kTag = "RTOS_IPC_Q";
        const std::uint32_t nullProbe = 0x55AAu;
        std::uint32_t nullOut = 0u;

        if (safeQueueSend(nullptr, &nullProbe, 0) != pdFAIL ||
            safeQueueReceive(nullptr, &nullOut, 0) != pdFAIL)
        {
            Log::sys_error(kTag, "Queue null-handle guard failed");
            return false;
        }

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

        // After freeing one slot from a full queue, sending must succeed again.
        if (xQueueSend(queue, &fourth, pdMS_TO_TICKS(10)) != pdTRUE)
        {
            ok = false;
            Log::sys_error(kTag, "Queue full-to-free transition failed");
        }

        ok = (xQueueReceive(queue, &out, pdMS_TO_TICKS(50)) == pdTRUE && out == second) && ok;
        ok = (xQueueReceive(queue, &out, pdMS_TO_TICKS(50)) == pdTRUE && out == third) && ok;
        ok = (xQueueReceive(queue, &out, pdMS_TO_TICKS(50)) == pdTRUE && out == fourth) && ok;

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
