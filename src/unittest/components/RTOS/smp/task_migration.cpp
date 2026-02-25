#include "./task_migration.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        struct MigrationContext
        {
            TaskHandle_t parent;
            volatile int coreSeen;
        };

        void migrationWorker(void *param)
        {
            auto *context = static_cast<MigrationContext *>(param);
            context->coreSeen = xPortGetCoreID();
            xTaskNotifyGive(context->parent);
            vTaskDelete(nullptr);
        }
    }

    bool runRtosTaskMigrationTest()
    {
        constexpr const char *kTag = "RTOS_SMP_MIG";

        MigrationContext context{};
        context.parent = xTaskGetCurrentTaskHandle();
        context.coreSeen = -1;

        if (xTaskCreate(migrationWorker, "smp_mig", 3072, &context, tskIDLE_PRIORITY + 1, nullptr) != pdPASS)
        {
            Log::sys_error(kTag, "Failed to create migration task");
            return false;
        }

        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(500)) == 0)
        {
            Log::sys_error(kTag, "Task migration timing out");
            return false;
        }

        if (context.coreSeen < 0)
        {
            Log::sys_error(kTag, "Task migration core sample invalid");
            return false;
        }

        Log::sys_info(kTag, "Task migration baseline successful");
        return true;
    }
} // namespace UnitTest
