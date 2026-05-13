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
#include "esp_timer.h"

#include "../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        constexpr const char *kTag = "RTOSTEST";

        struct SuiteResult
        {
            const char *name;
            bool passed;
            std::int64_t durationUs;
        };
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

        constexpr std::size_t kSuiteCount = 20u;
        SuiteResult results[kSuiteCount]{};
        std::size_t resultCount = 0u;

        auto runAndRecord = [&](const char *name, bool (*suiteFn)())
        {
            const std::int64_t startUs = esp_timer_get_time();
            const bool passed = suiteFn();
            const std::int64_t durationUs = esp_timer_get_time() - startUs;

            if (resultCount < kSuiteCount)
            {
                results[resultCount++] = SuiteResult{name, passed, durationUs};
            }

            const char *status = passed ? "PASS" : "FAIL";
            Log::sys_info(kTag, String("[") + status + "] " + name + ", duration-us=" + String(static_cast<long long>(durationUs)));
            return passed;
        };

        bool ok = true;
        ok = runAndRecord("scheduler", runRtosSchedulerTest) && ok;
        ok = runAndRecord("determinism", runRtosDeterminismTest) && ok;
        ok = runAndRecord("timing", runRtosTimingTest) && ok;
        ok = runAndRecord("interrupts", runRtosInterruptSuite) && ok;
        ok = runAndRecord("synchronization", runRtosSynchronizationSuite) && ok;
        ok = runAndRecord("deadlock_starvation", runRtosDeadlockStarvationSuite) && ok;
        ok = runAndRecord("memory_management", runRtosMemoryManagementSuite) && ok;
        ok = runAndRecord("ipc", runRtosIpcSuite) && ok;
        ok = runAndRecord("power_management", runRtosPowerManagementSuite) && ok;
        ok = runAndRecord("smp", runRtosSmpSuite) && ok;
        ok = runAndRecord("fault_injection", runRtosFaultInjectionSuite) && ok;
        ok = runAndRecord("stress", runRtosStressSuite) && ok;
        ok = runAndRecord("concurrency", runRtosConcurrencyTest) && ok;
        ok = runAndRecord("interrupt_behavior", runRtosInterruptBehaviorTest) && ok;
        ok = runAndRecord("safety_critical", runRtosSafetyCriticalSuite) && ok;
        ok = runAndRecord("port_layer", runRtosPortLayerSuite) && ok;
        ok = runAndRecord("hardware_vs_simulation", runRtosHardwareVsSimulationSuite) && ok;
        ok = runAndRecord("coverage", runRtosCoverageSuite) && ok;
        ok = runAndRecord("fuzz", runRtosFuzzSuite) && ok;
        ok = runAndRecord("formal_verification", runRtosFormalVerificationSuite) && ok;

        std::size_t passedCount = 0u;
        std::size_t failedCount = 0u;
        std::int64_t totalDurationUs = 0;
        String failedSuites;

        for (std::size_t i = 0u; i < resultCount; ++i)
        {
            totalDurationUs += results[i].durationUs;
            if (results[i].passed)
            {
                ++passedCount;
                continue;
            }

            ++failedCount;
            if (!failedSuites.isEmpty())
            {
                failedSuites += ",";
            }
            failedSuites += results[i].name;
        }

        if (!ok)
        {
            Log::sys_error(
                kTag,
                "One or more RTOS component tests failed, total=" + String(resultCount) +
                    ", passed=" + String(passedCount) +
                    ", failed=" + String(failedCount) +
                    ", total-duration-us=" + String(static_cast<long long>(totalDurationUs)) +
                    ", failed-suites=" + failedSuites);
            return false;
        }

        Log::sys_info(
            kTag,
            "RTOS test successful, scheduler=" + String(static_cast<int>(schedulerState)) +
                ", tasks-before=" + String(tasksBefore) +
                ", tasks-after=" + String(tasksAfter) +
                ", prio=" + String(currentPriority) +
                ", suites=" + String(resultCount) +
                ", total-duration-us=" + String(static_cast<long long>(totalDurationUs)));
        return true;
    }
} // namespace UnitTest
