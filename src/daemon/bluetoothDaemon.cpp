#include "daemon.h"
#include "../config/config.h"
#include "../connectivity/bluetooth/bluetooth.h"
#include "../serial/log.h"

static const char *PRIV_DAEMON_TAG = "BT_DAEMON";

namespace
{
    BluetoothManager g_bluetoothManager;

    constexpr size_t kMaxBleNameLength = 20;

    String clampBleName(const String &name)
    {
        if (name.length() <= kMaxBleNameLength)
        {
            return name;
        }
        return name.substring(0, kMaxBleNameLength);
    }

    String buildBluetoothName()
    {
        String deviceIdPart = DEVICE_ID >= 0 ? String(DEVICE_ID) : String("NA");
        String baseName = "BLE_" + deviceIdPart + "-" + String(TEAMID);
        if (String(DEVICE_NAME).length() > 0)
        {
            baseName += "+";
            baseName += String(DEVICE_NAME);
        }
        return clampBleName(baseName);
    }

    void bluetoothDaemonTask(void *pvParameters)
    {
        Log::sys_info(PRIV_DAEMON_TAG, "Bluetooth daemon started on Core " + String(xPortGetCoreID()));

        String appliedName = "";

        while (true)
        {
            const String targetName = buildBluetoothName();

            if (!g_bluetoothManager.isInitialized())
            {
                if (g_bluetoothManager.begin(targetName))
                {
                    g_bluetoothManager.startServer();
                    appliedName = targetName;
                    Log::sys_info(PRIV_DAEMON_TAG, "Bluetooth initialized with name: " + targetName);
                }
                else
                {
                    Log::sys_error(PRIV_DAEMON_TAG, "Bluetooth initialization failed");
                }
            }
            else if (targetName != appliedName)
            {
                Log::sys_info(PRIV_DAEMON_TAG, "Bluetooth name update required: " + targetName);
                g_bluetoothManager.end();

                if (g_bluetoothManager.begin(targetName))
                {
                    g_bluetoothManager.startServer();
                    appliedName = targetName;
                    Log::sys_info(PRIV_DAEMON_TAG, "Bluetooth renamed to: " + targetName);
                }
                else
                {
                    Log::sys_error(PRIV_DAEMON_TAG, "Bluetooth reinitialization failed");
                }
            }

            delay(5000);
        }
    }
}

void Daemon::startBluetoothDaemon(void)
{
    Log::sys_info(PRIV_DAEMON_TAG, "Starting bluetooth daemon...");

    xTaskCreatePinnedToCore(
        bluetoothDaemonTask,
        "BluetoothDaemon",
        8192,
        NULL,
        1,
        NULL,
        1);

    Log::sys_info(PRIV_DAEMON_TAG, "Bluetooth daemon started");
}
