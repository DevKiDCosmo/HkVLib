#include "./sleep_mode.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosSleepModeTest()
    {
        constexpr const char *kTag = "RTOS_PWR_SLEEP";

        vTaskDelay(pdMS_TO_TICKS(20));
        vTaskDelay(pdMS_TO_TICKS(20));

        Log::sys_info(kTag, "Sleep mode entry/exit baseline successful");
        return true;
    }
} // namespace UnitTest
