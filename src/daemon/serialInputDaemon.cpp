#include "daemon.h"
#include "../config/config.h"
#include "esp_log.h"
#include "../connectivity/wifi/wifi.h"
#include "../serial/log.h"
#include "../serial/commands/debug.h"

static const char *PRIV_DAEMON_TAG = "SERIAL";

void serialInputDaemonTask(void *pvParameters)
{

    Log::sys_infoflag(PRIV_DAEMON_TAG, "Serial input daemon started on Core " + String(xPortGetCoreID()), DEBUG_FLAG_EXTENSIVE);

    while (true)
    {
        if (Serial.available() > 0)
        {
            String input = Serial.readStringUntil('\n');
            input.trim();
            Log::sys_infoflag(PRIV_DAEMON_TAG, "Received command: " + input, DEBUG_SERIAL);

            // Process commands here
            if (input.equalsIgnoreCase("wifi status"))
            {
                if (g_wifi != nullptr)
                {
                    Log::sys_info(PRIV_DAEMON_TAG, "WiFi Status: " + String(g_wifi->isConnected() ? "Connected" : "Disconnected") + ", IP: " + (g_wifi->isConnected() ? g_wifi->getLocalIP() : "N/A"));
                }
                else
                {
                    Log::sys_warning(PRIV_DAEMON_TAG, "WiFi not initialized");
                }
            }
            else if (input.equalsIgnoreCase("reboot"))
            {
                Log::sys_info(PRIV_DAEMON_TAG, "Rebooting system...");
                esp_restart();
            }
            else if (input.equalsIgnoreCase("rtos tasks") || input.equalsIgnoreCase("daemon status"))
            {
                SerialDebugCommands::RTOSBgTask();
            }
            else if (input.equalsIgnoreCase("display ping") || input.equalsIgnoreCase("display test"))
            {
                SerialDebugCommands::DisplayPing();
            }
            else if (input.startsWith("daemon notify"))
            {
                String taskName = input.substring(String("daemon notify").length());
                taskName.trim();
                SerialDebugCommands::DaemonNotify(taskName);
            }
        }

        delay(100); // Check for input every 100ms
    }
}

void Daemon::startSerialInputDaemon(void)
{
    Log::sys_info(PRIV_DAEMON_TAG, "Starting serial input daemon...");

    // Create a simple serial input task on Core 1
    xTaskCreatePinnedToCore(
        serialInputDaemonTask,
        "SerialInputDaemon",
        4096,
        NULL,
        1,
        NULL,
        0); // Priority 1 on Core 0. Higher priority than main loop to ensure responsiveness.

    Log::sys_info(PRIV_DAEMON_TAG, "Serial input daemon started");
}