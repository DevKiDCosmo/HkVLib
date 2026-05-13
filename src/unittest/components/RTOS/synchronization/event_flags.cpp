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
        constexpr EventBits_t both = bitA | bitB;

        EventGroupHandle_t group = xEventGroupCreate();
        if (group == nullptr)
        {
            Log::sys_error(kTag, "Failed to create event group");
            return false;
        }

        xEventGroupSetBits(group, bitA);
        const EventBits_t partial = xEventGroupWaitBits(group, both, pdFALSE, pdTRUE, pdMS_TO_TICKS(20));
        if ((partial & both) == both)
        {
            vEventGroupDelete(group);
            Log::sys_error(kTag, "Partial bits unexpectedly satisfied all-bits wait");
            return false;
        }

        xEventGroupSetBits(group, bitB);
        const EventBits_t got = xEventGroupWaitBits(group, both, pdFALSE, pdTRUE, pdMS_TO_TICKS(100));
        vEventGroupDelete(group);

        if ((got & both) != both)
        {
            Log::sys_error(kTag, "Event flags wait/set failed");
            return false;
        }

        Log::sys_info(kTag, "Event flags baseline successful");
        return true;
    }
} // namespace UnitTest
