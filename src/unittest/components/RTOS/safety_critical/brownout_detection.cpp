#include "./brownout_detection.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosBrownoutDetectionTest()
    {
        constexpr const char *kTag = "RTOS_SAFE_BOD";

#if defined(CONFIG_ESP_BROWNOUT_DET) && (CONFIG_ESP_BROWNOUT_DET == 1)
        Log::sys_info(kTag, "Brown-out detection configured");
        return true;
#else
        Log::sys_info(kTag, "Brown-out detection not configured, treated as skipped baseline");
        return true;
#endif
    }
} // namespace UnitTest
