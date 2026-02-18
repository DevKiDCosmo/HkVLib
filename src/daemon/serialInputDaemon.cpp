#include "daemon.h"
#include "../config/config.h"
#include "esp_log.h"
#include "../connectivity/wifi/wifi.h"

static const char *PRIV_DAEMON_TAG = "SERIAL";

void serialInputDaemonTask(void *pvParameters)
{
    ESP_LOGI(PRIV_DAEMON_TAG, "Serial input daemon started on Core %d", xPortGetCoreID());

    while (true)
    {
        if (Serial.available() > 0)
        {
            String input = Serial.readStringUntil('\n');
            input.trim();
            ESP_LOGI(PRIV_DAEMON_TAG, "Received command: %s", input.c_str());

            // Process commands here
            if (input.equalsIgnoreCase("wifi status"))
            {
                if (g_wifi != nullptr)
                {
                    ESP_LOGI(PRIV_DAEMON_TAG, "WiFi Status: %s, IP: %s",
                             g_wifi->isConnected() ? "Connected" : "Disconnected",
                             g_wifi->isConnected() ? g_wifi->getLocalIP().c_str() : "N/A");
                }
                else
                {
                    ESP_LOGW(PRIV_DAEMON_TAG, "WiFi not initialized");
                }
            }
            else if (input.equalsIgnoreCase("reboot"))
            {
                ESP_LOGI(PRIV_DAEMON_TAG, "Rebooting system...");
                esp_restart();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Check for input every 100ms
    }
}

void Daemon::startSerialInputDaemon(void)
{
    ESP_LOGI(PRIV_DAEMON_TAG, "Starting serial input daemon...");

    // Create a simple serial input task on Core 1
    xTaskCreatePinnedToCore(
        serialInputDaemonTask,
        "SerialInputDaemon",
        4096,
        NULL,
        1,
        NULL,
        1);
}