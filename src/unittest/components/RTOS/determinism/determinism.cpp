#include "./determinism.h"

#include "./nondeterministic_guard.h"
#include "./schedule_repeatability.h"
#include "./timing_repeatability.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosDeterminismTest()
    {
        constexpr const char *kTag = "RTOS_DET";

        bool ok = true;
        ok = runRtosScheduleRepeatabilityTest() && ok;
        ok = runRtosTimingRepeatabilityTest() && ok;
        ok = runRtosNondeterministicBehaviorGuardTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more determinism tests failed");
            return false;
        }

        Log::sys_info(kTag, "Determinism tests successful");
        return true;
    }
} // namespace UnitTest
