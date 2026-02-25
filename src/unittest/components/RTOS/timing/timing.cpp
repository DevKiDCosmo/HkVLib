#include "./timing.h"

#include "./deadline.h"
#include "./jitter.h"
#include "./tick_accuracy.h"
#include "./wcet.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosTimingSuite()
    {
        constexpr const char *kTag = "RTOS_TIME";

        bool ok = true;
        ok = runRtosTaskExecutionDeadlineTest() && ok;
        ok = runRtosTickAccuracyTest() && ok;
        ok = runRtosJitterMeasurementTest() && ok;
        ok = runRtosWcetTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more timing tests failed");
            return false;
        }

        Log::sys_info(kTag, "Timing guarantees baseline successful");
        return true;
    }
} // namespace UnitTest
