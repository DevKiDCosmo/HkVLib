#include "./static_dynamic_memory_model.h"

#include "freertos/FreeRTOS.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosFormalStaticDynamicMemoryModelTest()
    {
        constexpr const char *kTag = "RTOS_FORMAL_MEM";

#if (configSUPPORT_STATIC_ALLOCATION == 1) || (configSUPPORT_DYNAMIC_ALLOCATION == 1)
        Log::sys_info(
            kTag,
            "Memory model configuration available: static=" + String(configSUPPORT_STATIC_ALLOCATION) +
                ", dynamic=" + String(configSUPPORT_DYNAMIC_ALLOCATION));
        return true;
#else
        Log::sys_error(kTag, "Neither static nor dynamic allocation is enabled");
        return false;
#endif
    }
} // namespace UnitTest
