#include "./deadlock_starvation.h"

#include "./circular_wait.h"
#include "./priority_inversion.h"
#include "./starvation.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosDeadlockStarvationSuite()
    {
        constexpr const char *kTag = "RTOS_DL";

        bool ok = true;
        ok = runRtosCircularWaitTest() && ok;
        ok = runRtosPriorityInversionTest() && ok;
        ok = runRtosStarvationSimulationTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more deadlock/starvation tests failed");
            return false;
        }

        Log::sys_info(kTag, "Deadlock/starvation tests successful");
        return true;
    }
} // namespace UnitTest
