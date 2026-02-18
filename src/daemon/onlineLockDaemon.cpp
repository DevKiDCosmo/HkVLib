#include "../config/config.h"
#include "../onlinelock/onlinelock.h"
#include "../include.h"
#include "daemon.h"

static const char *PRIV_DAEMON_TAG = "ONLINE_LOCK";

// Online lock daemon - monitors connection and controls OnlineLock state
static void onlineLockDaemonTask(void *pvParameters)
{
    ESP_LOGI(PRIV_DAEMON_TAG, "Online lock daemon started on Core %d", xPortGetCoreID());
    vTaskDelay(pdMS_TO_TICKS(2000)); // Wait for initialization

    while (true)
    {
        bool is_connected = g_wifi != nullptr && g_wifi->isConnected();

        if (!is_connected && !OnlineLock::isLocked())
        {
            // Lost internet - engage lock to interrupt operations
            OnlineLock::engageLock();
        }
        else if (is_connected && OnlineLock::isLocked())
        {
            // Internet restored - disengage lock to resume operations
            OnlineLock::disengageLock();
        }

        vTaskDelay(pdMS_TO_TICKS(5000)); // Check every 5 seconds
    }
}

// Start online lock daemon
void Daemon::startOnlineLockDaemon(void)
{
    esp_log_level_set(PRIV_DAEMON_TAG, ESP_LOG_INFO);

    ESP_LOGI(PRIV_DAEMON_TAG, "Starting online lock daemon...");
    xTaskCreatePinnedToCore(
        onlineLockDaemonTask,
        "OnlineLockDaemon",
        4096,
        NULL,
        1,
        NULL,
        1 // Core 1
    );
    ESP_LOGI(PRIV_DAEMON_TAG, "Online lock daemon started");
}