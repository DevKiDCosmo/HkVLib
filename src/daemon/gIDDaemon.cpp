#include "daemon.h"
#include "../config/config.h"
#include "../network/request.h"
#include "../serial/log.h"
#include <Arduino.h>

static const char *PRIV_DAEMON_TAG = "gIDDaemon";

void gIDDaemonTask(void *pvParameters)
{
    Log::sys_info(PRIV_DAEMON_TAG, "gID Heartbeat daemon started on Core " + String(xPortGetCoreID()));

    HttpRequest httpClient(SERVER, PORT);
    while (true)
    {
        String macStr = MAC_ADDR;
        String endpoint = "/heartbeat/" + macStr + "/" + String(DEVICE_ID);
        HttpResponse response = httpClient.get(endpoint);

        if (response.success)
        {
            Log::sys_info(PRIV_DAEMON_TAG, "Heartbeat gID sent successfully - Status: " + String(response.statusCode) + " Response: " + response.body);
        }
        else
        {
            Log::sys_error(PRIV_DAEMON_TAG, "Heartbeat gID failed - Status: " + String(response.statusCode));
        }

        delay(1000 * HEARTBEAT_DEVICE_AVAIBILITY); // 10 second delay
    }
}

void Daemon::startgIDDaemon(void)
{
    Log::sys_info(PRIV_DAEMON_TAG, "Starting gID daemon...");

    // Create a simple gID task on Core 1
    xTaskCreatePinnedToCore(
        gIDDaemonTask,
        "gIDDaemon",
        8192,
        NULL,
        1,
        NULL,
        1);

    Log::sys_info(PRIV_DAEMON_TAG, "Heartbeat daemon started");
}