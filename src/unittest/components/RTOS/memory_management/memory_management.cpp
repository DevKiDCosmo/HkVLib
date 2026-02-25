#include "./memory_management.h"

#include "./dynamic_allocation.h"
#include "./memory_protection.h"
#include "./static_allocation.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosMemoryManagementSuite()
    {
        constexpr const char *kTag = "RTOS_MEM";

        bool ok = true;
        ok = runRtosStaticAllocationTest() && ok;
        ok = runRtosDynamicAllocationTest() && ok;
        ok = runRtosMemoryProtectionTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more memory management tests failed");
            return false;
        }

        Log::sys_info(kTag, "Memory management tests successful");
        return true;
    }
} // namespace UnitTest
