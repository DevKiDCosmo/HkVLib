#include "./safety_critical.h"

#include "./brownout_detection.h"
#include "./memory_safety.h"
#include "./safe_state_fatal_error.h"
#include "./stack_overflow_hook.h"
#include "./watchdog_integration.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosSafetyCriticalSuite()
    {
        constexpr const char *kTag = "RTOS_SAFE";

        bool ok = true;
        ok = runRtosStackOverflowHookTest() && ok;
        ok = runRtosWatchdogIntegrationTest() && ok;
        ok = runRtosSafeStateOnFatalErrorTest() && ok;
        ok = runRtosBrownoutDetectionTest() && ok;
        ok = runRtosMemorySafetyTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more safety-critical tests failed");
            return false;
        }

        Log::sys_info(kTag, "Safety-critical tests successful");
        return true;
    }
} // namespace UnitTest
