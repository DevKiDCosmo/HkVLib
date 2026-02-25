#include "./event_flags.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosEventFlagsTest()
    {
        constexpr const char *kTag = "RTOS_SYNC_EVT";
        constexpr EventBits_t bitA = (1u << 0);
        constexpr EventBits_t bitB = (1u << 1);

        EventGroupHandle_t group = xEventGroupCreate();
        if (group == nullptr)
        {
            Log::sys_error(kTag, "Failed to create event group");
            return false;
        }

        xEventGroupSetBits(group, bitA | bitB);
        const EventBits_t got = xEventGroupWaitBits(group, bitA | bitB, pdFALSE, pdTRUE, pdMS_TO_TICKS(100));
        vEventGroupDelete(group);

        if ((got & (bitA | bitB)) != (bitA | bitB))
        {
            Log::sys_error(kTag, "Event flags wait/set failed");
            return false;
        }

        Log::sys_info(kTag, "Event flags baseline successful");
        return true;
    }
} // namespace UnitTest
