#include "../config/config.h"
#include "../onlinelock/onlinelock.h"
#include "../include.h"
#include "daemon.h"
#include "../serial/log.h"

static const char *PRIV_DAEMON_TAG = "ONLINE_LOCK";

// Online lock daemon - monitors connection and controls OnlineLock state
static void onlineLockDaemonTask(void *pvParameters)
{
    Log::sys_info(PRIV_DAEMON_TAG, "Online lock daemon started on Core " + String(xPortGetCoreID()));
    delay(2000); // Wait for initialization

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

        delay(5000); // Check every 5 seconds
    }
}

// Start online lock daemon
void Daemon::startOnlineLockDaemon(void)
{
    Log::sys_info(PRIV_DAEMON_TAG, "Starting online lock daemon...");
    xTaskCreatePinnedToCore(
        onlineLockDaemonTask,
        "OnlineLockDaemon",
        4096,
        NULL,
        1,
        NULL,
        1 // Core 1
    );
    Log::sys_info(PRIV_DAEMON_TAG, "Online lock daemon started");
}