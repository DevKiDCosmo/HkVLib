#include "unittest/psram.h"

#include <cstddef>
#include <cstdint>

#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "serial/log.h"

namespace UnitTest
{
    namespace
    {
        constexpr const char *kTag = "PSRAMTEST";
        constexpr std::size_t kInitialChunkSize = 256u * 1024u;
        constexpr std::size_t kMinChunkSize = 4u * 1024u;
        constexpr std::size_t kPageStep = 4096u;

        struct BlockHeader
        {
            BlockHeader *next;
            std::size_t totalSize;
            std::size_t index;
        };

        inline std::uint8_t pattern(std::size_t blockIdx, std::size_t offset, std::uint8_t seed)
        {
            return static_cast<std::uint8_t>((blockIdx * 173u + offset / kPageStep + seed) & 0xFFu);
        }

        void writePattern(std::uint8_t *mem, std::size_t size, std::size_t blockIdx, std::uint8_t seed)
        {
            std::size_t step = 0;
            for (std::size_t off = 0; off < size; off += kPageStep)
            {
                mem[off] = pattern(blockIdx, off, seed);
                ++step;
                if ((step & 0x7Fu) == 0u)
                {
                    vTaskDelay(1);
                }
            }
            mem[size - 1] = pattern(blockIdx, size - 1, seed);
        }

        bool verifyPattern(const std::uint8_t *mem, std::size_t size, std::size_t blockIdx, std::uint8_t seed)
        {
            std::size_t step = 0;
            for (std::size_t off = 0; off < size; off += kPageStep)
            {
                if (mem[off] != pattern(blockIdx, off, seed))
                {
                    Log::sys_error(kTag, "PSRAM Error at Block " + String(blockIdx) + ", Offset " + String(off));
                    return false;
                }

                ++step;
                if ((step & 0x7Fu) == 0u)
                {
                    vTaskDelay(1);
                }
            }

            if (mem[size - 1] != pattern(blockIdx, size - 1, seed))
            {
                Log::sys_error(kTag, "PSRAM Error at Block " + String(blockIdx) + ", last byte");
                return false;
            }

            return true;
        }

        bool addPsramBlock(BlockHeader *&head, std::size_t blockIndex, std::size_t chunkSize)
        {
            auto *raw = static_cast<std::uint8_t *>(heap_caps_malloc(chunkSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
            if (!raw)
            {
                return false;
            }

            BlockHeader *header = reinterpret_cast<BlockHeader *>(raw);
            header->next = head;
            header->totalSize = chunkSize;
            header->index = blockIndex;

            std::uint8_t *payload = raw + sizeof(BlockHeader);
            const std::size_t payloadSize = chunkSize - sizeof(BlockHeader);
            writePattern(payload, payloadSize, blockIndex, 0x3Cu);

            head = header;
            return true;
        }

        void freeAllPsram(BlockHeader *head)
        {
            while (head)
            {
                BlockHeader *next = head->next;
                heap_caps_free(head);
                head = next;
            }
        }
    } // namespace

    bool runPsramTest()
    {
        const std::size_t total = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
        const std::size_t freeBefore = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);

        if (total == 0u)
        {
            Log::sys_error(kTag, "No PSRAM detected or PSRAM not enabled");
            return false;
        }

        if (kInitialChunkSize <= sizeof(BlockHeader) || kMinChunkSize <= sizeof(BlockHeader))
        {
            Log::sys_error(kTag, "Invalid PSRAM test configuration");
            return false;
        }

        BlockHeader *head = nullptr;
        std::size_t blockIndex = 0;
        std::size_t totalBytes = 0;

        std::size_t chunkSize = kInitialChunkSize;
        while (chunkSize >= kMinChunkSize)
        {
            bool allocatedAtThisSize = false;
            while (addPsramBlock(head, blockIndex, chunkSize))
            {
                allocatedAtThisSize = true;
                totalBytes += chunkSize;
                ++blockIndex;
                if ((blockIndex & 0x0Fu) == 0u)
                {
                    vTaskDelay(1);
                }
            }

            if (!allocatedAtThisSize)
            {
                chunkSize /= 2u;
            }
        }

        if (blockIndex == 0u)
        {
            Log::sys_error(kTag, "No PSRAM block could be allocated");
            return false;
        }

        for (BlockHeader *it = head; it != nullptr; it = it->next)
        {
            const std::size_t payloadSize = it->totalSize - sizeof(BlockHeader);
            std::uint8_t *payload = reinterpret_cast<std::uint8_t *>(it) + sizeof(BlockHeader);

            if (!verifyPattern(payload, payloadSize, it->index, 0x3Cu))
            {
                freeAllPsram(head);
                return false;
            }

            writePattern(payload, payloadSize, it->index, 0xC3u);
            if (!verifyPattern(payload, payloadSize, it->index, 0xC3u))
            {
                freeAllPsram(head);
                return false;
            }
        }

        freeAllPsram(head);

        const std::size_t testedKiB = totalBytes / 1024u;
        Log::sys_info(
            kTag,
            "PSRAM test successful, total=" + String(total / 1024u) +
                " KiB, free-before=" + String(freeBefore / 1024u) +
                " KiB, tested=" + String(testedKiB) + " KiB");
        return true;
    }
} // namespace UnitTest
