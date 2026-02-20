#include "daemon.h"
#include "../config/config.h"
#include "../connectivity/wifi/wifi.h"
#include "../serial/log.h"
#include <Arduino.h>

static const char *PRIV_DAEMON_TAG = "NET_DAEMON";

// Network daemon task - runs asynchronously on Core 0
static void networkDaemonTask(void *pvParameters)
{
    Log::sys_infoflag(PRIV_DAEMON_TAG, "Network daemon started on Core " + String(xPortGetCoreID()), DEBUG_FLAG_EXTENSIVE);

    // Wait a moment for initialization to complete
    delay(1000);

    while (true)
    {
        //? Check if backup creedentials are used if login with backup were once succesful.
        if (g_wifi != nullptr && g_ssid.length() > 0 && g_password.length() > 0)
        {
            // Check WiFi status periodically
            if (!g_wifi->isConnected())
            {
                Log::sys_warning(PRIV_DAEMON_TAG, "WiFi disconnected! Attempting reconnect...");
                if (g_wifi->connect(g_ssid, g_password))
                {
                    Log::sys_info(PRIV_DAEMON_TAG, "WiFi reconnected. IP: " + g_wifi->getLocalIP());
                }
                else
                {
                    Log::sys_error(PRIV_DAEMON_TAG, "WiFi reconnect failed!");
                }
            }
            else
            {
                // WiFi is connected - just log status occasionally
                static int counter = 0;
                if (++counter % 12 == 0)
                { // Every 60 seconds
                    Log::sys_info(PRIV_DAEMON_TAG, "WiFi OK - IP: " + g_wifi->getLocalIP());
                    counter = 0;
                }
            }
        }

        // Check every 5 seconds
        delay(WLAN_PRIORITY * 1000); // Convert seconds to milliseconds
    }
}

void Daemon::startNetworkDaemon(void)
{
    Log::sys_info(PRIV_DAEMON_TAG, "Starting network daemon...");

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

    Log::sys_info(PRIV_DAEMON_TAG, "Network daemon started");
}
