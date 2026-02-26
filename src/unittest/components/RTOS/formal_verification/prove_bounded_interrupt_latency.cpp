#include "./prove_bounded_interrupt_latency.h"

#include "../interrupts/interrupt_latency.h"

namespace UnitTest
{
    bool runRtosFormalBoundedInterruptLatencyTest()
    {
        return runRtosInterruptLatencyTest();
    }
} // namespace UnitTest
