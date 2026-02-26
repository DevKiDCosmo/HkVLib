#include "./safety_critical_target.h"

#include "../safety_critical/safety_critical.h"

namespace UnitTest
{
    bool runRtosFormalSafetyCriticalTargetTest()
    {
        return runRtosSafetyCriticalSuite();
    }
} // namespace UnitTest
