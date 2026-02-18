#include "daemon.h"
#include "../config/config.h"
#include "../network/request.h"
#include "../serial/log.h"
#include <Arduino.h>

static const char *PRIV_DAEMON_TAG = "HEARTBEAT";

static void heartbeatDaemonTask(void *pvParameters)
{
    Log::sys_info(PRIV_DAEMON_TAG, "Heartbeat daemon started on Core " + String(xPortGetCoreID()));

    // Initialize HTTP client with server configuration
    HttpRequest httpClient(SERVER, PORT);

    while (true)
    {

        Log::sys_infoflag(PRIV_DAEMON_TAG, "System is alive - Free heap: " + String(esp_get_free_heap_size()) + " bytes", DEBUG_FLAG_EXTENSIVE);

        // Send heartbeat request to SERVER:PORT/heartbeat
        HttpResponse response = httpClient.get("/heartbeat");
        if (response.success)
        {
            Log::sys_info(PRIV_DAEMON_TAG, "Heartbeat sent successfully - Status: " + String(response.statusCode) + " Response: " + response.body);
        }
        else
        {
            Log::sys_error(PRIV_DAEMON_TAG, "Heartbeat failed - Status: " + String(response.statusCode));
        }

        delay(1000 * HEARTBEAT_SERVER_AVAIBILITY); // Every 30 seconds
    }
}

void Daemon::startHeartbeatDaemon(void)
{
    Log::sys_info(PRIV_DAEMON_TAG, "Starting heartbeat daemon...");

    // Create a simple heartbeat task on Core 1
    xTaskCreatePinnedToCore(
        heartbeatDaemonTask,
        "HeartbeatDaemon",
        8192,
        NULL,
        1,
        NULL,
        1);

    Log::sys_info(PRIV_DAEMON_TAG, "Heartbeat daemon started");
}