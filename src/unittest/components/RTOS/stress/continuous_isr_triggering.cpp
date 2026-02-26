#include "./continuous_isr_triggering.h"

#include "../fault_injection/isr_storm.h"

namespace UnitTest
{
    bool runRtosContinuousIsrTriggeringStressTest()
    {
        return runRtosIsrStormTest();
    }
} // namespace UnitTest
