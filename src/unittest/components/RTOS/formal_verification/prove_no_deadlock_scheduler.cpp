#include "./prove_no_deadlock_scheduler.h"

#include "../deadlock_starvation/deadlock_starvation.h"

namespace UnitTest
{
    bool runRtosFormalNoDeadlockSchedulerTest()
    {
        return runRtosDeadlockStarvationSuite();
    }
} // namespace UnitTest
