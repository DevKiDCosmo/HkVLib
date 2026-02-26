#include "./formal_verification.h"

#include "./model_check_scheduling.h"
#include "./prove_bounded_interrupt_latency.h"
#include "./prove_no_deadlock_scheduler.h"
#include "./prove_priority_inheritance_correctness.h"
#include "./safety_critical_target.h"
#include "./static_dynamic_memory_model.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosFormalVerificationSuite()
    {
        constexpr const char *kTag = "RTOS_FORMAL";

        bool ok = true;
        ok = runRtosFormalNoDeadlockSchedulerTest() && ok;
        ok = runRtosFormalBoundedInterruptLatencyTest() && ok;
        ok = runRtosFormalPriorityInheritanceCorrectnessTest() && ok;
        ok = runRtosFormalModelCheckSchedulingTest() && ok;
        ok = runRtosFormalStaticDynamicMemoryModelTest() && ok;
        ok = runRtosFormalSafetyCriticalTargetTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more formal verification proxy tests failed");
            return false;
        }

        Log::sys_info(kTag, "Formal verification proxy tests successful");
        return true;
    }
} // namespace UnitTest
