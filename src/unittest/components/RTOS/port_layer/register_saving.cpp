#include "./register_saving.h"

#include "../scheduler/context_switch_integrity.h"

namespace UnitTest
{
    bool runRtosPortRegisterSavingTest()
    {
        return runRtosContextSwitchIntegrityTest();
    }
} // namespace UnitTest
