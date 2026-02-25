#include "./static_allocation.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosStaticAllocationTest()
    {
        constexpr const char *kTag = "RTOS_MEM_STA";
        const UBaseType_t stackWords = uxTaskGetStackHighWaterMark(nullptr);

        if (stackWords < 128u)
        {
            Log::sys_error(kTag, "Stack boundary too low: " + String(stackWords) + " words");
            return false;
        }

        Log::sys_info(kTag, "Static allocation baseline successful");
        return true;
    }
} // namespace UnitTest
