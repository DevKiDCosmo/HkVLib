#include "./memory_safety.h"

#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../serial/log.h"

namespace UnitTest
{
    bool runRtosMemorySafetyTest()
    {
        constexpr const char *kTag = "RTOS_MEM";

        const UBaseType_t stackWords = uxTaskGetStackHighWaterMark(nullptr);
        if (stackWords < 128u)
        {
            Log::sys_error(kTag, "Low stack headroom: " + String(stackWords) + " words");
            return false;
        }

        const std::size_t heapBefore = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        void *buffer = heap_caps_malloc(1024u, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if (buffer == nullptr)
        {
            Log::sys_error(kTag, "Heap allocation failed");
            return false;
        }

        heap_caps_free(buffer);
        const std::size_t heapAfter = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if (heapAfter + 256u < heapBefore)
        {
            Log::sys_error(kTag, "Possible heap leak detected: before=" + String(heapBefore) + ", after=" + String(heapAfter));
            return false;
        }

        Log::sys_info(kTag, "Memory safety baseline successful");
        return true;
    }
} // namespace UnitTest
