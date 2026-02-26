#include "./error_branch_coverage.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosErrorBranchCoverageTest()
    {
        constexpr const char *kTag = "RTOS_COV_ERR";

        QueueHandle_t queue = xQueueCreate(1, sizeof(std::uint32_t));
        if (queue == nullptr)
        {
            Log::sys_error(kTag, "Queue creation failed");
            return false;
        }

        const std::uint32_t first = 1u;
        const std::uint32_t second = 2u;

        if (xQueueSend(queue, &first, 0) != pdTRUE)
        {
            Log::sys_error(kTag, "Queue initial send failed");
            vQueueDelete(queue);
            return false;
        }

        if (xQueueSend(queue, &second, 0) == pdTRUE)
        {
            Log::sys_error(kTag, "Queue overflow error branch was not hit");
            vQueueDelete(queue);
            return false;
        }

        vQueueDelete(queue);
        Log::sys_info(kTag, "Error branch coverage baseline successful");
        return true;
    }
} // namespace UnitTest
