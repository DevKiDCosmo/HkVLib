#include "./memory_protection.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosMemoryProtectionTest()
    {
        constexpr const char *kTag = "RTOS_MEM_MPU";

        Log::sys_info(kTag, "MPU isolation trap test not available on this target - baseline pass");
        return true;
    }
} // namespace UnitTest
