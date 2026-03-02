#include "daemon.h"
#include "../config/config.h"
#include "esp_log.h"
#include "../connectivity/wifi/wifi.h"
#include "../serial/log.h"
#include "../serial/commands/command_registry.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../nvs/nvs.h"

#include <cstdint>

static const char *PRIV_DAEMON_TAG = "SERIAL";

void serialInputDaemonTask(void *pvParameters);

namespace
{
    constexpr UBaseType_t kSerialDaemonPriority = 1u;
    constexpr uint32_t kSerialDaemonStackSize = 4096u;
    constexpr BaseType_t kSerialDaemonCore = 0;

    constexpr UBaseType_t kSerialSupervisorPriority = 2u;
    constexpr uint32_t kSerialSupervisorStackSize = 3072u;
    constexpr BaseType_t kSerialSupervisorCore = 0;

    constexpr TickType_t kSerialHealthCheckPeriod = pdMS_TO_TICKS(1000);
    constexpr TickType_t kSerialStallTimeout = pdMS_TO_TICKS(5000);

    TaskHandle_t g_serialDaemonTaskHandle = nullptr;
    TaskHandle_t g_serialSupervisorTaskHandle = nullptr;
    volatile TickType_t g_serialDaemonHeartbeatTick = 0;

    void updateSerialHeartbeat()
    {
        g_serialDaemonHeartbeatTick = xTaskGetTickCount();
    }

    TickType_t readSerialHeartbeat()
    {
        return g_serialDaemonHeartbeatTick;
    }

    bool startSerialInputDaemonTask(const char *reason)
    {
        if (g_serialDaemonTaskHandle != nullptr)
        {
            const eTaskState state = eTaskGetState(g_serialDaemonTaskHandle);
            if (state != eDeleted)
            {
                return true;
            }

            g_serialDaemonTaskHandle = nullptr;
        }

        updateSerialHeartbeat();
        const BaseType_t created = xTaskCreatePinnedToCore(
            serialInputDaemonTask,
            "SerialInputDaemon",
            kSerialDaemonStackSize,
            nullptr,
            kSerialDaemonPriority,
            &g_serialDaemonTaskHandle,
            kSerialDaemonCore);

        if (created != pdPASS || g_serialDaemonTaskHandle == nullptr)
        {
            Log::sys_error(PRIV_DAEMON_TAG, "Failed to start serial input daemon, reason=" + String(reason));
            g_serialDaemonTaskHandle = nullptr;
            return false;
        }

        Log::sys_info(PRIV_DAEMON_TAG, "Serial input daemon started, reason=" + String(reason));
        return true;
    }

    void serialSupervisorTask(void *pvParameters)
    {
        (void)pvParameters;
        Log::sys_info(PRIV_DAEMON_TAG, "Serial supervisor started on Core " + String(xPortGetCoreID()));

        while (true)
        {
            const TickType_t now = xTaskGetTickCount();
            const TickType_t heartbeat = readSerialHeartbeat();

            bool needsRestart = false;
            if (g_serialDaemonTaskHandle == nullptr)
            {
                needsRestart = true;
            }
            else
            {
                const eTaskState state = eTaskGetState(g_serialDaemonTaskHandle);
                if (state == eDeleted)
                {
                    g_serialDaemonTaskHandle = nullptr;
                    needsRestart = true;
                }
                else if ((now - heartbeat) > kSerialStallTimeout)
                {
                    Log::sys_error(PRIV_DAEMON_TAG, "Serial daemon stalled, restarting...");
                    vTaskDelete(g_serialDaemonTaskHandle);
                    g_serialDaemonTaskHandle = nullptr;
                    needsRestart = true;
                }
            }

            if (needsRestart)
            {
                startSerialInputDaemonTask("supervisor");
            }

            vTaskDelay(kSerialHealthCheckPeriod);
        }
    }
} // namespace

void serialInputDaemonTask(void *pvParameters)
{
    (void)pvParameters;

    Log::sys_infoflag(PRIV_DAEMON_TAG, "Serial input daemon started on Core " + String(xPortGetCoreID()), DEBUG_FLAG_EXTENSIVE);

    while (true)
    {
        updateSerialHeartbeat();

        if (Serial.available() > 0)
        {
            String input = Serial.readStringUntil('\n');
            input.trim();
            Log::sys_infoflag(PRIV_DAEMON_TAG, "Received command: " + input, DEBUG_SERIAL);

            if (!SerialCommandRegistry::dispatch(input))
            {
                Log::sys_warning(PRIV_DAEMON_TAG, "Unknown command: " + input + " (try 'help')");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Check for input every 100ms
    }
}

void Daemon::startSerialInputDaemon(void)
{
    Log::sys_info(PRIV_DAEMON_TAG, "Starting serial input daemon...");

    Serial.setTimeout(50);
    startSerialInputDaemonTask("initial");

    if (g_serialSupervisorTaskHandle == nullptr)
    {
        const BaseType_t created = xTaskCreatePinnedToCore(
            serialSupervisorTask,
            "SerialSupervisor",
            kSerialSupervisorStackSize,
            nullptr,
            kSerialSupervisorPriority,
            &g_serialSupervisorTaskHandle,
            kSerialSupervisorCore);

        if (created != pdPASS)
        {
            Log::sys_error(PRIV_DAEMON_TAG, "Failed to start serial supervisor");
            g_serialSupervisorTaskHandle = nullptr;
            return;
        }
    }

    Log::sys_info(PRIV_DAEMON_TAG, "Serial daemon supervision active");
}