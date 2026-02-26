#include "./isr_coverage.h"

#include "../interrupts/interrupt_behavior.h"
#include "../interrupts/interrupts.h"

namespace UnitTest
{
    bool runRtosIsrCoverageTest()
    {
        bool ok = true;
        ok = runRtosInterruptSuite() && ok;
        ok = runRtosInterruptBehaviorTest() && ok;
        return ok;
    }
} // namespace UnitTest
