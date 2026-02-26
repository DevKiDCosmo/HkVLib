#include "./stack_overflow_hook.h"

#include "freertos/FreeRTOS.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosStackOverflowHookTest()
    {
        constexpr const char *kTag = "RTOS_SAFE_STK";

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
        Log::sys_info(kTag, "Stack overflow hook configuration enabled");
        return true;
#else
        Log::sys_error(kTag, "configCHECK_FOR_STACK_OVERFLOW disabled");
        return false;
#endif
    }
} // namespace UnitTest
