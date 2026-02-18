#include "daemon.h"
#include "esp_log.h"
#include "../config/config.h"
#include "../network/request.h"
#include <Arduino.h>

const char *PRIV_DAEMON_TAG = "gIDDaemon";

void gIDDaemonTask(void *pvParameters)
{
    ESP_LOGI(PRIV_DAEMON_TAG, "gID Heartbeat daemon started on Core %d", xPortGetCoreID());

    HttpRequest httpClient(SERVER, PORT);
    while (true)
    {
        String macStr = MAC_ADDR;
        String endpoint = "/id/" + macStr + "/" + String(TEAMID);
        HttpResponse response = httpClient.get(endpoint);

        if (response.success)
        {
            ESP_LOGI(PRIV_DAEMON_TAG, "Heartbeat gID sent successfully - Status: %d, Response: %s",
                     response.statusCode, response.body.c_str());
        }
        else
        {
            ESP_LOGE(PRIV_DAEMON_TAG, "Heartbeat gID failed - Status: %d", response.statusCode);
        }

        vTaskDelay(pdMS_TO_TICKS(1000 * HEARTBEAT_DEVICE_AVAIBILITY)); // 10 second delay
    }
}

void Daemon::startgIDDaemon(void)
{
    ESP_LOGI(PRIV_DAEMON_TAG, "Starting gID daemon...");

    // Create a simple gID task on Core 1
    xTaskCreatePinnedToCore(
        gIDDaemonTask,
        "gIDDaemon",
        8192,
        NULL,
        1,
        NULL,
        1);

    ESP_LOGI(PRIV_DAEMON_TAG, "Heartbeat daemon started");
}