#include "./stress.h"

#include "./continuous_isr_triggering.h"
#include "./maximum_queue_usage.h"
#include "./rapid_create_delete.h"
#include "./tasks_100_plus.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosStressSuite()
    {
        constexpr const char *kTag = "RTOS_STR";

        bool ok = true;
        ok = runRtos100PlusTasksStressTest() && ok;
        ok = runRtosRapidCreateDeleteStressTest() && ok;
        ok = runRtosMaximumQueueUsageStressTest() && ok;
        ok = runRtosContinuousIsrTriggeringStressTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more stress tests failed");
            return false;
        }

        Log::sys_info(kTag, "Stress tests successful");
        return true;
    }
} // namespace UnitTest
