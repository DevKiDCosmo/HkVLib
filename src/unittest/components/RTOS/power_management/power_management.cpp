#include "./power_management.h"

#include "./sleep_mode.h"
#include "./tickless_idle.h"
#include "./timekeeping.h"
#include "./wake_source.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosPowerManagementSuite()
    {
        constexpr const char *kTag = "RTOS_PWR";

        bool ok = true;
        ok = runRtosTicklessIdleTest() && ok;
        ok = runRtosSleepModeTest() && ok;
        ok = runRtosWakeSourceTest() && ok;
        ok = runRtosTimekeepingAfterSleepTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more power management tests failed");
            return false;
        }

        Log::sys_info(kTag, "Power management tests successful");
        return true;
    }
} // namespace UnitTest
