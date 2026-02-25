#include "./synchronization.h"

#include "./condition_variables.h"
#include "./event_flags.h"
#include "./mutex.h"
#include "./semaphore.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosSynchronizationSuite()
    {
        constexpr const char *kTag = "RTOS_SYNC";

        bool ok = true;
        ok = runRtosMutexTest() && ok;
        ok = runRtosSemaphoreTest() && ok;
        ok = runRtosEventFlagsTest() && ok;
        ok = runRtosConditionVariableTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more synchronization tests failed");
            return false;
        }

        Log::sys_info(kTag, "Synchronization primitive tests successful");
        return true;
    }
} // namespace UnitTest
