#include "./target_power_behavior.h"

#include "../power_management/power_management.h"

namespace UnitTest
{
    bool runRtosTargetPowerBehaviorTest()
    {
        return runRtosPowerManagementSuite();
    }
} // namespace UnitTest
