#include "./RTOS.h"
#include "./coverage/coverage.h"
#include "./concurrency/concurrency.h"
#include "./deadlock_starvation/deadlock_starvation.h"
#include "./determinism/determinism.h"
#include "./fault_injection/fault_injection.h"
#include "./formal_verification/formal_verification.h"
#include "./fuzz/fuzz.h"
#include "./hardware_simulation/hardware_simulation.h"
#include "./ipc/ipc.h"
#include "./interrupts/interrupts.h"
#include "./interrupts/interrupt_behavior.h"
#include "./memory_management/memory_management.h"
#include "./port_layer/port_layer.h"
#include "./power_management/power_management.h"
#include "./safety_critical/safety_critical.h"
#include "./scheduler/scheduler.h"
#include "./smp/smp.h"
#include "./stress/stress.h"
#include "./synchronization/synchronization.h"
#include "./timing/timing_entry.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        constexpr const char *kTag = "RTOSTEST";
    }

    bool runRtosTest()
    {
        const BaseType_t schedulerState = xTaskGetSchedulerState();
        if (schedulerState == taskSCHEDULER_NOT_STARTED)
        {
            Log::sys_error(kTag, "Scheduler not started");
            return false;
        }

        TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();
        if (currentTask == nullptr)
        {
            Log::sys_error(kTag, "Current task handle is null");
            return false;
        }

        const UBaseType_t tasksBefore = uxTaskGetNumberOfTasks();
        const UBaseType_t currentPriority = uxTaskPriorityGet(currentTask);

        if (currentPriority >= configMAX_PRIORITIES)
        {
            Log::sys_error(kTag, "Current task priority out of range: " + String(currentPriority));
            return false;
        }

        vTaskDelay(1);

        const UBaseType_t tasksAfter = uxTaskGetNumberOfTasks();
        if (tasksAfter == 0u)
        {
            Log::sys_error(kTag, "No RTOS tasks reported after scheduler tick");
            return false;
        }

        bool ok = true;
        ok = runRtosSchedulerTest() && ok;
        ok = runRtosDeterminismTest() && ok;
        ok = runRtosTimingTest() && ok;
        ok = runRtosInterruptSuite() && ok;
        ok = runRtosSynchronizationSuite() && ok;
        ok = runRtosDeadlockStarvationSuite() && ok;
        ok = runRtosMemoryManagementSuite() && ok;
        ok = runRtosIpcSuite() && ok;
        ok = runRtosPowerManagementSuite() && ok;
        ok = runRtosSmpSuite() && ok;
        ok = runRtosFaultInjectionSuite() && ok;
        ok = runRtosStressSuite() && ok;
        ok = runRtosConcurrencyTest() && ok;
        ok = runRtosInterruptBehaviorTest() && ok;
        ok = runRtosSafetyCriticalSuite() && ok;
        ok = runRtosPortLayerSuite() && ok;
        ok = runRtosHardwareVsSimulationSuite() && ok;
        ok = runRtosCoverageSuite() && ok;
        ok = runRtosFuzzSuite() && ok;
        ok = runRtosFormalVerificationSuite() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more RTOS component tests failed");
            return false;
        }

        Log::sys_info(
            kTag,
            "RTOS test successful, scheduler=" + String(static_cast<int>(schedulerState)) +
                ", tasks-before=" + String(tasksBefore) +
                ", tasks-after=" + String(tasksAfter) +
                ", prio=" + String(currentPriority));
        return true;
    }
} // namespace UnitTest
