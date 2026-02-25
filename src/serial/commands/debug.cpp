#include "debug.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../display/display.h"

namespace
{
    struct DaemonNotifyContext
    {
        String taskName;
        uint32_t timeoutMs;
    };

    const char *taskStateToString(eTaskState state)
    {
        switch (state)
        {
        case eRunning:
            return "Running";
        case eReady:
            return "Ready";
        case eBlocked:
            return "Blocked";
        case eSuspended:
            return "Susp";
        case eDeleted:
            return "Deleted";
        case eInvalid:
        default:
            return "Invalid";
        }
    }

#if defined(configUSE_TRACE_FACILITY) && (configUSE_TRACE_FACILITY == 1)
    bool getTaskStateByName(const String &taskName, eTaskState &stateOut)
    {
        const UBaseType_t taskCount = uxTaskGetNumberOfTasks();
        if (taskCount == 0)
        {
            return false;
        }

        TaskStatus_t *taskStatusArray = static_cast<TaskStatus_t *>(pvPortMalloc(taskCount * sizeof(TaskStatus_t)));
        if (taskStatusArray == nullptr)
        {
            return false;
        }

        const UBaseType_t currentTaskCount = uxTaskGetSystemState(taskStatusArray, taskCount, nullptr);
        bool found = false;
        for (UBaseType_t index = 0; index < currentTaskCount; ++index)
        {
            const TaskStatus_t &task = taskStatusArray[index];
            if (taskName.equalsIgnoreCase(task.pcTaskName))
            {
                stateOut = task.eCurrentState;
                found = true;
                break;
            }
        }

        vPortFree(taskStatusArray);
        return found;
    }

    void daemonNotifyTask(void *parameter)
    {
        DaemonNotifyContext *context = static_cast<DaemonNotifyContext *>(parameter);
        if (context == nullptr)
        {
            vTaskDelete(nullptr);
            return;
        }

        const TickType_t startTick = xTaskGetTickCount();
        const TickType_t timeoutTicks = pdMS_TO_TICKS(context->timeoutMs);

        bool seenNonRunning = false;
        eTaskState state = eInvalid;
        if (getTaskStateByName(context->taskName, state))
        {
            seenNonRunning = (state != eRunning);
            if (!seenNonRunning)
            {
                Serial.printf("daemon notify: task '%s' is already running.\n", context->taskName.c_str());
                delete context;
                vTaskDelete(nullptr);
                return;
            }
        }

        while (true)
        {
            if (getTaskStateByName(context->taskName, state))
            {
                if (state != eRunning)
                {
                    seenNonRunning = true;
                }
                else if (seenNonRunning)
                {
                    Serial.printf("daemon notify: task '%s' is running again.\n", context->taskName.c_str());
                    break;
                }
            }

            if (context->timeoutMs > 0 && (xTaskGetTickCount() - startTick) >= timeoutTicks)
            {
                Serial.printf("daemon notify: timeout waiting for '%s' to run again.\n", context->taskName.c_str());
                break;
            }

            vTaskDelay(pdMS_TO_TICKS(200));
        }

        delete context;
        vTaskDelete(nullptr);
    }
#endif
} // namespace

void SerialDebugCommands::RTOSBgTask()
{
    const UBaseType_t taskCount = uxTaskGetNumberOfTasks();
    if (taskCount == 0)
    {
        Serial.println("No FreeRTOS tasks found.");
        return;
    }

#if defined(configUSE_TRACE_FACILITY) && (configUSE_TRACE_FACILITY == 1)
    TaskStatus_t *taskStatusArray = static_cast<TaskStatus_t *>(pvPortMalloc(taskCount * sizeof(TaskStatus_t)));
    if (taskStatusArray == nullptr)
    {
        Serial.println("Failed to allocate memory for task list.");
        return;
    }

    uint32_t totalRuntime = 0;
    const UBaseType_t currentTaskCount = uxTaskGetSystemState(taskStatusArray, taskCount, &totalRuntime);
    if (currentTaskCount == 0)
    {
        vPortFree(taskStatusArray);
        Serial.println("Failed to fetch system task state.");
        return;
    }

    Serial.println("\n=== RTOS Task / Daemon Status ===");
    Serial.printf("%-4s %-20s %-9s %-5s %-6s %-8s %-8s %-11s %-8s\n",
                  "ID", "Name", "State", "Core", "Prio", "BasePr", "StackHW", "Runtime", "CPU%");
    Serial.println("----------------------------------------------------------------------------------------------");

    for (UBaseType_t index = 0; index < currentTaskCount; ++index)
    {
        const TaskStatus_t &task = taskStatusArray[index];
        const double cpuPercent = (totalRuntime > 0)
                                      ? (100.0 * static_cast<double>(task.ulRunTimeCounter) / static_cast<double>(totalRuntime))
                                      : 0.0;

        const char *coreString = "N/A";

        Serial.printf("%-4lu %-20s %-9s %-5s %-6lu %-8lu %-8u %-11lu %7.2f\n",
                      static_cast<unsigned long>(task.xTaskNumber),
                      task.pcTaskName,
                      taskStateToString(task.eCurrentState),
                      coreString,
                      static_cast<unsigned long>(task.uxCurrentPriority),
                      static_cast<unsigned long>(task.uxBasePriority),
                      static_cast<unsigned int>(task.usStackHighWaterMark),
                      static_cast<unsigned long>(task.ulRunTimeCounter),
                      cpuPercent);
    }

    Serial.printf("Total tasks: %lu\n", static_cast<unsigned long>(currentTaskCount));
    vPortFree(taskStatusArray);
#else
    Serial.println("\n=== RTOS Task / Daemon Status ===");
    Serial.printf("%-4s %-20s %-9s %-5s %-6s %-8s %-8s %-11s %-8s\n",
                  "ID", "Name", "State", "Core", "Prio", "BasePr", "StackHW", "Runtime", "CPU%");
    Serial.println("----------------------------------------------------------------------------------------------");
    Serial.printf("%-4s %-20s %-9s %-5s %-6s %-8s %-8s %-11s %-8s\n",
                  "-", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A");
    Serial.printf("Total tasks: %lu\n", static_cast<unsigned long>(taskCount));
    Serial.println("Hint: enable FreeRTOS trace facility to list all tasks in detail.");
#endif
}

void SerialDebugCommands::DisplayPing()
{
    if (!Display::is_ready())
    {
        Serial.println("Display is not ready.");
        return;
    }

    const uint16_t white = Display::color(cyber, Display::PresetColor::White);
    const uint16_t red = Display::color(cyber, Display::PresetColor::Red);
    const uint16_t green = Display::color(cyber, Display::PresetColor::Green);
    const uint16_t blue = Display::color(cyber, Display::PresetColor::Blue);

    cyber.clean_lcd();

    for (uint8_t x = 0; x < 128; ++x)
    {
        cyber.set_lcd_pixel(x, 0, white);
        cyber.set_lcd_pixel(x, 127, white);
        if ((x % 2) == 0)
        {
            cyber.set_lcd_pixel(x, 63, blue);
        }
    }

    for (uint8_t y = 0; y < 128; ++y)
    {
        cyber.set_lcd_pixel(0, y, white);
        cyber.set_lcd_pixel(127, y, white);
        if ((y % 2) == 0)
        {
            cyber.set_lcd_pixel(63, y, green);
        }
    }

    for (uint8_t i = 8; i < 120; ++i)
    {
        cyber.set_lcd_pixel(i, i, red);
        cyber.set_lcd_pixel(127 - i, i, red);
    }

    cyber.render_lcd();
    delay(30);

    Display::draw_log(cyber, "DISPLAY PING OK", Display::PresetColor::Yellow);
    Serial.println("display ping rendered");
}

void SerialDebugCommands::DaemonNotify(const String &daemonTask)
{
    const String taskName = daemonTask;
    if (taskName.length() == 0)
    {
        Serial.println("Usage: daemon notify <taskname>");
        return;
    }

#if defined(configUSE_TRACE_FACILITY) && (configUSE_TRACE_FACILITY == 1)
    DaemonNotifyContext *context = new DaemonNotifyContext();
    if (context == nullptr)
    {
        Serial.println("daemon notify: failed to allocate watcher context.");
        return;
    }

    context->taskName = taskName;
    context->timeoutMs = 120000;

    BaseType_t created = xTaskCreatePinnedToCore(
        daemonNotifyTask,
        "DaemonNotify",
        4096,
        context,
        1,
        nullptr,
        0);

    if (created != pdPASS)
    {
        delete context;
        Serial.println("daemon notify: failed to start watcher task.");
        return;
    }

    Serial.printf("daemon notify: watching '%s' for running-again event (120s timeout).\n", taskName.c_str());
#else
    Serial.println("daemon notify unavailable: FreeRTOS trace facility disabled.");
#endif
}