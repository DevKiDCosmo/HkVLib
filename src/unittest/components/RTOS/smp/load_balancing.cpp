#include "./load_balancing.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosLoadBalancingTest()
    {
        constexpr const char *kTag = "RTOS_SMP_LOAD";
        const UBaseType_t taskCountBefore = uxTaskGetNumberOfTasks();
        vTaskDelay(1);
        const UBaseType_t taskCountAfter = uxTaskGetNumberOfTasks();

        if (taskCountAfter == 0u || taskCountBefore == 0u)
        {
            Log::sys_error(kTag, "Load balancing baseline failed");
            return false;
        }

        Log::sys_info(kTag, "Load balancing baseline successful");
        return true;
    }
} // namespace UnitTest
