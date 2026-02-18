#include "daemon.h"
#include "../config/config.h"
#include "../connectivity/wifi/wifi.h"
#include "esp_log.h"
#include <Arduino.h>

static const char *PRIV_DAEMON_TAG = "NET_DAEMON";

// Network daemon task - runs asynchronously on Core 0
static void networkDaemonTask(void *pvParameters)
{
    ESP_LOGI(PRIV_DAEMON_TAG, "Network daemon started on Core %d", xPortGetCoreID());

    // Wait a moment for initialization to complete
    vTaskDelay(pdMS_TO_TICKS(1000));

    while (true)
    {
        if (g_wifi != nullptr && g_ssid.length() > 0 && g_password.length() > 0)
        {
            // Check WiFi status periodically
            if (!g_wifi->isConnected())
            {
                ESP_LOGW(PRIV_DAEMON_TAG, "WiFi disconnected! Attempting reconnect...");
                if (g_wifi->connect(g_ssid, g_password))
                {
                    ESP_LOGI(PRIV_DAEMON_TAG, "WiFi reconnected. IP: %s", g_wifi->getLocalIP().c_str());
                }
                else
                {
                    ESP_LOGE(PRIV_DAEMON_TAG, "WiFi reconnect failed!");
                }

                // TODO: ONLINE_LOCK logic
            }
            else
            {
                // WiFi is connected - just log status occasionally
                static int counter = 0;
                if (++counter % 12 == 0)
                { // Every 60 seconds
                    ESP_LOGI(PRIV_DAEMON_TAG, "WiFi OK - IP: %s", g_wifi->getLocalIP().c_str());
                    counter = 0;
                }
            }
        }

        // Check every 5 seconds
        vTaskDelay(pdMS_TO_TICKS(WLAN_PRIORITY * 1000)); // Convert seconds to milliseconds
    }
}

void Daemon::startNetworkDaemon(void)
{
    ESP_LOGI(PRIV_DAEMON_TAG, "Starting network daemon...");

    // Create network daemon task on Core 0
    // Stack size: 4096 bytes, Priority: 1 (low), Core: 0
    xTaskCreatePinnedToCore(
        networkDaemonTask, // Task function
        "NetworkDaemon",   // Task name
        4096,              // Stack size
        NULL,              // Parameter
        1,                 // Priority
        NULL,              // Task handle
        0                  // Core 0
    );

    ESP_LOGI(PRIV_DAEMON_TAG, "Network daemon started");
}
