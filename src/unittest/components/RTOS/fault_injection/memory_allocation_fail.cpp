#include "./memory_allocation_fail.h"

#include <cstddef>

#include "esp_heap_caps.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosForcedMemoryAllocationFailTest()
    {
        constexpr const char *kTag = "RTOS_FI_MEM";
        const std::size_t freeInternal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        void *pointer = heap_caps_malloc(freeInternal + 1024u, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

        if (pointer != nullptr)
        {
            heap_caps_free(pointer);
            Log::sys_error(kTag, "Forced allocation fail test failed");
            return false;
        }

        Log::sys_info(kTag, "Forced allocation fail test successful");
        return true;
    }
} // namespace UnitTest
