#include "./memory_allocation_fail.h"

#include <cstddef>
#include <limits>

#include "esp_heap_caps.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosForcedMemoryAllocationFailTest()
    {
        constexpr const char *kTag = "RTOS_FI_MEM";
        constexpr std::size_t kExcessAllocation = 1024u;
        const std::size_t freeInternal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

        std::size_t requestedSize = freeInternal;
        if (requestedSize > (std::numeric_limits<std::size_t>::max() - kExcessAllocation))
        {
            requestedSize = std::numeric_limits<std::size_t>::max();
        }
        else
        {
            requestedSize += kExcessAllocation;
        }

        void *pointer = heap_caps_malloc(requestedSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

        if (pointer != nullptr)
        {
            heap_caps_free(pointer);
            Log::sys_error(
                kTag,
                "Forced allocation fail test failed: requested=" + String(static_cast<long long>(requestedSize)) +
                    ", free-internal=" + String(static_cast<long long>(freeInternal)));
            return false;
        }

        Log::sys_info(
            kTag,
            "Forced allocation fail test successful: requested=" + String(static_cast<long long>(requestedSize)) +
                ", free-internal=" + String(static_cast<long long>(freeInternal)));
        return true;
    }
} // namespace UnitTest
