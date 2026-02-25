#include "./mailbox.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosMailboxTest()
    {
        constexpr const char *kTag = "RTOS_IPC_MB";
        QueueHandle_t mailbox = xQueueCreate(1, sizeof(std::uint32_t));
        if (mailbox == nullptr)
        {
            Log::sys_error(kTag, "Failed to create mailbox queue");
            return false;
        }

        const std::uint32_t firstValue = 0x11u;
        const std::uint32_t overwriteValue = 0x22u;

        bool ok = true;
        ok = (xQueueOverwrite(mailbox, &firstValue) == pdPASS) && ok;
        ok = (xQueueOverwrite(mailbox, &overwriteValue) == pdPASS) && ok;

        std::uint32_t out = 0u;
        ok = (xQueueReceive(mailbox, &out, pdMS_TO_TICKS(20)) == pdTRUE && out == overwriteValue) && ok;

        if (xQueueReceive(mailbox, &out, pdMS_TO_TICKS(5)) == pdTRUE)
        {
            ok = false;
            Log::sys_error(kTag, "Mailbox blocking semantics failed");
        }

        vQueueDelete(mailbox);

        if (!ok)
        {
            return false;
        }

        Log::sys_info(kTag, "Mailbox test successful");
        return true;
    }
} // namespace UnitTest
