#include "./hardware_simulation.h"

#include "./host_ipc_logic.h"
#include "./host_memory_allocator.h"
#include "./host_scheduler_logic.h"
#include "./target_interrupt_latency.h"
#include "./target_power_behavior.h"
#include "./target_timing_precision.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosHardwareVsSimulationSuite()
    {
        constexpr const char *kTag = "RTOS_HW_SIM";

        bool ok = true;
        ok = runRtosHostSchedulerLogicTest() && ok;
        ok = runRtosHostIpcLogicTest() && ok;
        ok = runRtosHostMemoryAllocatorTest() && ok;
        ok = runRtosTargetTimingPrecisionTest() && ok;
        ok = runRtosTargetInterruptLatencyTest() && ok;
        ok = runRtosTargetPowerBehaviorTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more hardware/simulation tests failed");
            return false;
        }

        Log::sys_info(kTag, "Hardware vs simulation tests successful");
        return true;
    }
} // namespace UnitTest
