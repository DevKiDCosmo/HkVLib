#include "./context_switch_fail.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        void dummyTask(void *param)
        {
            (void)param;
            vTaskDelete(nullptr);
        }
    }

    bool runRtosForcedContextSwitchFailTest()
    {
        constexpr const char *kTag = "RTOS_FI_CTX";
        constexpr uint32_t kImpossibleStackWords = 512u * 1024u;
        TaskHandle_t handle = nullptr;

        const BaseType_t result = xTaskCreate(
            dummyTask,
            "fi_ctx_oom",
            kImpossibleStackWords,
            nullptr,
            tskIDLE_PRIORITY + 1,
            &handle);
        if (result == pdPASS)
        {
            if (handle != nullptr)
            {
                vTaskDelete(handle);
            }
            Log::sys_error(kTag, "Forced context switch failure path not triggered (task unexpectedly created)");
            return false;
        }

        Log::sys_info(kTag, "Forced context switch failure path successful (task creation rejected)");
        return true;
    }
} // namespace UnitTest
