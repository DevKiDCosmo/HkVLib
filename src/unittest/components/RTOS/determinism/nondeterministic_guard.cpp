#include "./nondeterministic_guard.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        std::uint32_t makeDeterministicChecksum()
        {
            std::uint32_t state = 0x1234ABCDu;
            for (int i = 0; i < 32; ++i)
            {
                state = (state * 1664525u) + 1013904223u;
                state ^= static_cast<std::uint32_t>(i);
                taskYIELD();
            }
            return state;
        }
    } // namespace

    bool runRtosNondeterministicBehaviorGuardTest()
    {
        constexpr const char *kTag = "RTOS_DET_ND";

        const std::uint32_t checksumA = makeDeterministicChecksum();
        const std::uint32_t checksumB = makeDeterministicChecksum();

        if (checksumA != checksumB)
        {
            Log::sys_error(kTag, "Nondeterministic behavior detected: " + String(checksumA) + " != " + String(checksumB));
            return false;
        }

        Log::sys_info(kTag, "Nondeterministic guard successful");
        return true;
    }
} // namespace UnitTest
