#include "./interrupts.h"

#include "./disable_enable_safety.h"
#include "./interrupt_latency.h"
#include "./isr_preemption.h"
#include "./isr_to_task_signaling.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosInterruptSuite()
    {
        constexpr const char *kTag = "RTOS_INT";

        bool ok = true;
        ok = runRtosIsrPreemptionTest() && ok;
        ok = runRtosInterruptLatencyTest() && ok;
        ok = runRtosIsrToTaskSignalingTest() && ok;
        ok = runRtosDisableEnableInterruptSafetyTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more interrupt tests failed");
            return false;
        }

        Log::sys_info(kTag, "Interrupt handling tests successful");
        return true;
    }
} // namespace UnitTest
