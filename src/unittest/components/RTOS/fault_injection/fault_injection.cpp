#include "./fault_injection.h"

#include "./artificial_delay.h"
#include "./context_switch_fail.h"
#include "./isr_storm.h"
#include "./memory_allocation_fail.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosFaultInjectionSuite()
    {
        constexpr const char *kTag = "RTOS_FI";

        bool ok = true;
        ok = runRtosIsrStormTest() && ok;
        ok = runRtosForcedMemoryAllocationFailTest() && ok;
        ok = runRtosForcedContextSwitchFailTest() && ok;
        ok = runRtosArtificialDelayInjectionTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more fault injection tests failed");
            return false;
        }

        Log::sys_info(kTag, "Fault injection tests successful");
        return true;
    }
} // namespace UnitTest
