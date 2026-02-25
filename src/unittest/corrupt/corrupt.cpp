#include "./corrupt.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "esp_heap_caps.h"

#include "../../serial/log.h"
#include "../../utility/init.h"

namespace UnitTest
{
    namespace Corrupt
    {
        namespace
        {
            constexpr const char *kTag = "UT_CORRUPT";
            constexpr std::size_t kChunkSize = 8u * 1024u;
            constexpr std::size_t kGuardBytes = 2u * 1024u;
            constexpr std::size_t kMaxChunks = 64u;

            bool gApplied = false;
            void *gReservedHeap[kMaxChunks] = {};
            std::size_t gReservedCount = 0u;
            std::size_t gReservedBytes = 0u;
        }

        void applyBeforeUnitTests()
        {
            if (gApplied)
            {
                return;
            }

            gApplied = true;
            Log::sys_warning(kTag, "CORRUPT_TEST enabled: injecting pre-unit-test corruption");

            if (Init::value() == 0)
            {
                Init::initialized();
                Log::sys_warning(kTag, "Injected state tamper: Init flag forced to initialized before init unit test");
            }

            while (gReservedCount < kMaxChunks)
            {
                const std::size_t freeInternal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
                if (freeInternal <= (kChunkSize + kGuardBytes))
                {
                    break;
                }

                void *block = heap_caps_malloc(kChunkSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
                if (block == nullptr)
                {
                    break;
                }

                std::memset(block, 0xA5, kChunkSize);
                gReservedHeap[gReservedCount++] = block;
                gReservedBytes += kChunkSize;
            }

            if (gReservedCount == 0u)
            {
                Log::sys_warning(kTag, "Heap pressure injection weak: no internal RAM blocks reserved");
                return;
            }

            const std::size_t remaining = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
            Log::sys_warning(kTag, "Injected aggressive heap pressure: reserved " + String(gReservedBytes / 1024u) +
                                       " KiB internal RAM in " + String(gReservedCount) +
                                       " blocks, remaining=" + String(remaining / 1024u) + " KiB");
        }

        void cleanupAfterUnitTests()
        {
            for (std::size_t i = 0; i < gReservedCount; ++i)
            {
                if (gReservedHeap[i] != nullptr)
                {
                    heap_caps_free(gReservedHeap[i]);
                    gReservedHeap[i] = nullptr;
                }
            }

            if (gReservedCount > 0u)
            {
                Log::sys_info(kTag, "Corruption cleanup: released reserved internal RAM blocks=" + String(gReservedCount));
            }

            gReservedCount = 0u;
            gReservedBytes = 0u;
        }
    } // namespace Corrupt
} // namespace UnitTest
