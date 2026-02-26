#include "./fuzz.h"

#include "./random_interrupt_timing.h"
#include "./random_semaphore_interactions.h"
#include "./random_task_creation_patterns.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosFuzzSuite()
    {
        constexpr const char *kTag = "RTOS_FUZZ";

        bool ok = true;
        ok = runRtosRandomTaskCreationPatternsFuzzTest() && ok;
        ok = runRtosRandomInterruptTimingFuzzTest() && ok;
        ok = runRtosRandomSemaphoreInteractionsFuzzTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more fuzz tests failed");
            return false;
        }

        Log::sys_info(kTag, "Fuzz tests successful");
        return true;
    }
} // namespace UnitTest
