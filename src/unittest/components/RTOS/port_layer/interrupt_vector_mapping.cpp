#include "./interrupt_vector_mapping.h"

#include "../interrupts/interrupt_latency.h"

namespace UnitTest
{
    bool runRtosPortInterruptVectorMappingTest()
    {
        return runRtosInterruptLatencyTest();
    }
} // namespace UnitTest
