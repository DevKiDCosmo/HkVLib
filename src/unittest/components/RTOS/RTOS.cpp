#include "./RTOS.h"
#include "./concurrency.h"
#include "./determinism.h"
#include "./interrupt_behavior.h"
#include "./memory_safety.h"
#include "./scheduler/scheduler.h"
#include "./timing.h"

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
        ok = runRtosConcurrencyTest() && ok;
        ok = runRtosInterruptBehaviorTest() && ok;
        ok = runRtosMemorySafetyTest() && ok;

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
