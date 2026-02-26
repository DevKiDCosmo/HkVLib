#include "daemon.h"
#include "../config/config.h"
#include "../network/request.h"
#include "../serial/log.h"
#include <Arduino.h>

static const char *PRIV_DAEMON_TAG = "gIDDaemon";

void gIDDaemonTask(void *pvParameters)
{
    Log::sys_info(PRIV_DAEMON_TAG, "gID Heartbeat daemon started on Core " + String(xPortGetCoreID()));

    HttpRequest httpClient;
    while (true)
    {
        // Check If ID is generated alreaddy. Else request ID
        if (DEVICE_ID == -1)
        {
            Log::sys_warning(PRIV_DAEMON_TAG, "Device ID not yet generated! Requesting new ID");

            Log::sys_info("DHCP_ID", "Requesting device ID from server...");

            // Send ID request to SERVER:PORT/id/:mac/:teamid
            String macStr = MAC_ADDR;
            String endpoint = "/id/" + macStr + "/" + String(TEAMID);
            HttpResponse response = httpClient.get(endpoint);

            if (response.success && response.statusCode == 200)
            {
                // Parse device ID from response body
                DEVICE_ID = response.body.toInt();
                String logMsg = "Device ID assigned: " + String(DEVICE_ID) + " (MAC: " + macStr + ", Team: " + String(TEAMID) + ")";
                Log::sys_info("DHCP_ID", logMsg);
            }
            else
            {
                Log::sys_error("DHCP_ID", "Failed to get device ID - Status: " + String(response.statusCode));
                DEVICE_ID = -1; // Indicate failure
            }

            delay(1000 * HEARTBEAT_DEVICE_AVAIBILITY);
            continue;
        }

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