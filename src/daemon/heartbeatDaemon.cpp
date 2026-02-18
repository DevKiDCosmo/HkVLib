#include "daemon.h"
#include "esp_log.h"
#include "../config/config.h"
#include "../network/request.h"
#include <Arduino.h>

static const char *PRIV_DAEMON_TAG = "HEARTBEAT";

static void heartbeatDaemonTask(void *pvParameters)
{
    ESP_LOGI(PRIV_DAEMON_TAG, "Heartbeat daemon started on Core %d", xPortGetCoreID());

    // Initialize HTTP client with server configuration
    HttpRequest httpClient(SERVER, PORT);

    while (true)
    {
        if (DEBUG_FLAG_EXTENSIVE)
        {
            ESP_LOGI(PRIV_DAEMON_TAG, "System is alive - Free heap: %d bytes", esp_get_free_heap_size());
        }

        // Send heartbeat request to SERVER:PORT/heartbeat
        HttpResponse response = httpClient.get("/heartbeat");
        if (response.success)
        {
            ESP_LOGI(PRIV_DAEMON_TAG, "Heartbeat sent successfully - Status: %d, Response: %s",
                     response.statusCode, response.body.c_str());
        }
        else
        {
            ESP_LOGE(PRIV_DAEMON_TAG, "Heartbeat failed - Status: %d", response.statusCode);
        }

        delay(1000 * HEARTBEAT_SERVER_AVAIBILITY); // Every 30 seconds
    }
}

void Daemon::startHeartbeatDaemon(void)
{
    ESP_LOGI(PRIV_DAEMON_TAG, "Starting heartbeat daemon...");

    // Create a simple heartbeat task on Core 1
    xTaskCreatePinnedToCore(
        heartbeatDaemonTask,
        "HeartbeatDaemon",
        8192,
        NULL,
        1,
        NULL,
        1);

    ESP_LOGI(PRIV_DAEMON_TAG, "Heartbeat daemon started");
}