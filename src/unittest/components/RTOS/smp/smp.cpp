#include "./smp.h"

#include "./cross_core_sync.h"
#include "./inter_core_interrupt.h"
#include "./load_balancing.h"
#include "./task_migration.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosSmpSuite()
    {
        constexpr const char *kTag = "RTOS_SMP";

        bool ok = true;
        ok = runRtosTaskMigrationTest() && ok;
        ok = runRtosLoadBalancingTest() && ok;
        ok = runRtosInterCoreInterruptTest() && ok;
        ok = runRtosCrossCoreSynchronizationTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more SMP tests failed");
            return false;
        }

        Log::sys_info(kTag, "SMP tests successful");
        return true;
    }
} // namespace UnitTest
