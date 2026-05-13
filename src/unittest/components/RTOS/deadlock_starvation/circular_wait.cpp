#include "./circular_wait.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        constexpr EventBits_t kTaskAReadyBit = (1u << 0);
        constexpr EventBits_t kTaskBReadyBit = (1u << 1);
        constexpr EventBits_t kBothReadyMask = kTaskAReadyBit | kTaskBReadyBit;

        struct CircularWaitTaskContext
        {
            SemaphoreHandle_t firstMutex;
            SemaphoreHandle_t secondMutex;
            EventGroupHandle_t barrier;
            TaskHandle_t parent;
            EventBits_t readyBit;
            volatile bool acquiredFirst;
            volatile bool timedOutOnSecond;
            volatile bool completed;
            volatile bool unexpectedSecondAcquire;
        };

        void circularWaitTask(void *param)
        {
            auto *context = static_cast<CircularWaitTaskContext *>(param);

            if (xSemaphoreTake(context->firstMutex, pdMS_TO_TICKS(200)) == pdTRUE)
            {
                context->acquiredFirst = true;

                xEventGroupSetBits(context->barrier, context->readyBit);
                const EventBits_t reached = xEventGroupWaitBits(
                    context->barrier,
                    kBothReadyMask,
                    pdFALSE,
                    pdTRUE,
                    pdMS_TO_TICKS(500));

                if ((reached & kBothReadyMask) == kBothReadyMask)
                {
                    if (xSemaphoreTake(context->secondMutex, pdMS_TO_TICKS(120)) == pdTRUE)
                    {
                        context->unexpectedSecondAcquire = true;
                        xSemaphoreGive(context->secondMutex);
                    }
                    else
                    {
                        context->timedOutOnSecond = true;
                    }
                }

                xSemaphoreGive(context->firstMutex);
            }

            context->completed = true;
            xTaskNotifyGive(context->parent);
            vTaskDelete(nullptr);
        }
    } // namespace

    bool runRtosCircularWaitTest()
    {
        constexpr const char *kTag = "RTOS_DL_CIRC";

        SemaphoreHandle_t a = xSemaphoreCreateMutex();
        SemaphoreHandle_t b = xSemaphoreCreateMutex();
        EventGroupHandle_t barrier = xEventGroupCreate();
        if (a == nullptr || b == nullptr || barrier == nullptr)
        {
            if (a != nullptr)
            {
                vSemaphoreDelete(a);
            }
            if (b != nullptr)
            {
                vSemaphoreDelete(b);
            }
            if (barrier != nullptr)
            {
                vEventGroupDelete(barrier);
            }
            Log::sys_error(kTag, "Failed to create synchronization primitives");
            return false;
        }

        TaskHandle_t parent = xTaskGetCurrentTaskHandle();
        CircularWaitTaskContext contextA{
            a,
            b,
            barrier,
            parent,
            kTaskAReadyBit,
            false,
            false,
            false,
            false};
        CircularWaitTaskContext contextB{
            b,
            a,
            barrier,
            parent,
            kTaskBReadyBit,
            false,
            false,
            false,
            false};

        TaskHandle_t taskA = nullptr;
        TaskHandle_t taskB = nullptr;

        if (xTaskCreate(circularWaitTask, "rtos_cw_a", 3072, &contextA, tskIDLE_PRIORITY + 2, &taskA) != pdPASS ||
            xTaskCreate(circularWaitTask, "rtos_cw_b", 3072, &contextB, tskIDLE_PRIORITY + 2, &taskB) != pdPASS)
        {
            if (taskA != nullptr)
            {
                vTaskDelete(taskA);
            }
            if (taskB != nullptr)
            {
                vTaskDelete(taskB);
            }

            vEventGroupDelete(barrier);
            vSemaphoreDelete(a);
            vSemaphoreDelete(b);
            Log::sys_error(kTag, "Failed to create circular wait tasks");
            return false;
        }

        for (int i = 0; i < 2; ++i)
        {
            if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(2000)) == 0u)
            {
                vTaskDelete(taskA);
                vTaskDelete(taskB);
                vEventGroupDelete(barrier);
                vSemaphoreDelete(a);
                vSemaphoreDelete(b);
                Log::sys_error(kTag, "Circular wait tasks did not complete in time");
                return false;
            }
        }

        const bool validCircularWait =
            contextA.completed &&
            contextB.completed &&
            contextA.acquiredFirst &&
            contextB.acquiredFirst &&
            contextA.timedOutOnSecond &&
            contextB.timedOutOnSecond &&
            !contextA.unexpectedSecondAcquire &&
            !contextB.unexpectedSecondAcquire;

        if (!validCircularWait)
        {
            vEventGroupDelete(barrier);
            vSemaphoreDelete(a);
            vSemaphoreDelete(b);
            Log::sys_error(
                kTag,
                "Circular wait scenario did not match expectations: "
                "a_first=" +
                    String(contextA.acquiredFirst ? 1 : 0) +
                    ", b_first=" + String(contextB.acquiredFirst ? 1 : 0) +
                    ", a_timeout_second=" + String(contextA.timedOutOnSecond ? 1 : 0) +
                    ", b_timeout_second=" + String(contextB.timedOutOnSecond ? 1 : 0));
            return false;
        }

        vEventGroupDelete(barrier);
        vSemaphoreDelete(a);
        vSemaphoreDelete(b);

        Log::sys_info(kTag, "Circular wait two-task timeout scenario successful");
        return true;
    }
} // namespace UnitTest
