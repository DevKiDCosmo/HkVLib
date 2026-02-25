#include "./scheduler.h"

#include "./context_switch_integrity.h"
#include "./preemption.h"
#include "./priority_scheduling.h"
#include "./round_robin.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosSchedulerTest()
    {
        constexpr const char *kTag = "RTOS_SCHED";

        bool ok = true;
        ok = runRtosPrioritySchedulingTest() && ok;
        ok = runRtosPreemptionTest() && ok;
        ok = runRtosRoundRobinTest() && ok;
        ok = runRtosContextSwitchIntegrityTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more scheduler tests failed");
            return false;
        }

        Log::sys_info(kTag, "Scheduler tests successful");
        return true;
    }
} // namespace UnitTest
