#include "./dynamic_allocation.h"

#include <cstdint>

#include "esp_heap_caps.h"
#include "esp_timer.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosDynamicAllocationTest()
    {
        constexpr const char *kTag = "RTOS_MEM_DYN";
        constexpr std::size_t kBlockSize = 256u;
        constexpr int kBlockCount = 48;
        constexpr std::int64_t kMaxAllocUs = 3000;

        void *blocks[kBlockCount] = {};
        std::int64_t maxAllocUs = 0;

        for (int i = 0; i < kBlockCount; ++i)
        {
            const std::int64_t start = esp_timer_get_time();
            blocks[i] = heap_caps_malloc(kBlockSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
            const std::int64_t elapsed = esp_timer_get_time() - start;

            if (elapsed > maxAllocUs)
            {
                maxAllocUs = elapsed;
            }

            if (blocks[i] == nullptr)
            {
                for (int j = 0; j < i; ++j)
                {
                    heap_caps_free(blocks[j]);
                }
                Log::sys_error(kTag, "Allocation stress failed at block " + String(i));
                return false;
            }
        }

        for (int i = 0; i < kBlockCount; i += 2)
        {
            heap_caps_free(blocks[i]);
            blocks[i] = nullptr;
        }

        for (int i = 0; i < kBlockCount; i += 2)
        {
            blocks[i] = heap_caps_malloc(kBlockSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
            if (blocks[i] == nullptr)
            {
                for (int j = 0; j < kBlockCount; ++j)
                {
                    if (blocks[j] != nullptr)
                    {
                        heap_caps_free(blocks[j]);
                    }
                }
                Log::sys_error(kTag, "Fragmentation recovery failed at block " + String(i));
                return false;
            }
        }

        for (int i = 0; i < kBlockCount; ++i)
        {
            heap_caps_free(blocks[i]);
        }

        if (maxAllocUs > kMaxAllocUs)
        {
            Log::sys_error(kTag, "Allocation time too high: " + String(static_cast<long long>(maxAllocUs)) + " us");
            return false;
        }

        Log::sys_info(kTag, "Dynamic allocation baseline successful");
        return true;
    }
} // namespace UnitTest
