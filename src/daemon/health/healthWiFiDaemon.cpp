#include "health.h"
#include "../../include.h"
#include "../../connectivity/wifi/wifi.h"

static const char *PRIV_DAEMON_TAG = "HealthWiFi";

void healthWiFiDaemonTask(void *pvParameters)
{
    Log::sys_info(PRIV_DAEMON_TAG, "Health WiFi daemon started on Core " + String(xPortGetCoreID()));

    while (true)
    {
        // Check if WiFi is connected
        if (!g_wifi->isConnected())
        {
            Log::sys_warning(PRIV_DAEMON_TAG, "WiFi not connected");
            health_setStatus(HEALTH_WIFI_CONNECTED, HEALTH_STATUS_ERROR);
            health_setStatus(HEALTH_WIFI_RSSI, HEALTH_STATUS_ERROR);
            health_setStatus(HEALTH_WIFI_TXPOWER, HEALTH_STATUS_ERROR);
            health_setStatus(HEALTH_WIFI_PING, HEALTH_STATUS_ERROR);
            health_setStatus(HEALTH_WIFI_CHANNEL, HEALTH_STATUS_ERROR);
            health_setStatus(HEALTH_WIFI_QUALITY, HEALTH_STATUS_ERROR);
            health_setStatus(HEALTH_WIFI_LINKSPEED, HEALTH_STATUS_ERROR);

            delay(1000 * HEALTH_WIFI_DAEMON); // Delay for the specified interval
            continue;
        }

        health_setStatus(HEALTH_WIFI_CONNECTED, HEALTH_STATUS_OK);

        // Get WiFi metrics
        int8_t rssi = g_wifi->getRSSI();
        uint8_t txPower = g_wifi->getTxPower();
        uint8_t channel = g_wifi->getChannel();
        int8_t quality = g_wifi->getSignalQuality();
        uint32_t pingMs = g_wifi->getPingLatency();
        String ssid = g_wifi->getSSID();
        String bssid = g_wifi->getBSSID();
        String localIP = g_wifi->getLocalIP();
        IPAddress gateway = g_wifi->getGatewayIP();

        Log::sys_info(PRIV_DAEMON_TAG, "WiFi Status - SSID: " + ssid + ", IP: " + localIP + ", Gateway: " + gateway.toString());
        Log::sys_info(PRIV_DAEMON_TAG, "WiFi Metrics - RSSI: " + String(rssi) + " dBm, Quality: " + String(quality) + "%, Channel: " + String(channel));
        Log::sys_info(PRIV_DAEMON_TAG, "WiFi Metrics - TX Power: " + String(txPower) + " dBm, BSSID: " + bssid);

        // Check RSSI (signal strength)
        if (rssi < -80)
        {
            Log::sys_error(PRIV_DAEMON_TAG, "WiFi RSSI critical: " + String(rssi) + " dBm");
            health_setStatus(HEALTH_WIFI_RSSI, HEALTH_STATUS_ERROR);
        }
        else if (rssi < -70)
        {
            Log::sys_warning(PRIV_DAEMON_TAG, "WiFi RSSI warning: " + String(rssi) + " dBm");
            health_setStatus(HEALTH_WIFI_RSSI, HEALTH_STATUS_WARNING);
        }
        else
        {
            health_setStatus(HEALTH_WIFI_RSSI, HEALTH_STATUS_OK);
        }

        // Check TX Power
        if (txPower < 10)
        {
            Log::sys_error(PRIV_DAEMON_TAG, "WiFi TX Power critical: " + String(txPower) + " dBm");
            health_setStatus(HEALTH_WIFI_TXPOWER, HEALTH_STATUS_ERROR);
        }
        else if (txPower < 15)
        {
            Log::sys_warning(PRIV_DAEMON_TAG, "WiFi TX Power warning: " + String(txPower) + " dBm");
            health_setStatus(HEALTH_WIFI_TXPOWER, HEALTH_STATUS_WARNING);
        }
        else
        {
            health_setStatus(HEALTH_WIFI_TXPOWER, HEALTH_STATUS_OK);
        }

        // Check Signal Quality
        if (quality < 30)
        {
            Log::sys_error(PRIV_DAEMON_TAG, "WiFi Quality critical: " + String(quality) + "%");
            health_setStatus(HEALTH_WIFI_QUALITY, HEALTH_STATUS_ERROR);
        }
        else if (quality < 50)
        {
            Log::sys_warning(PRIV_DAEMON_TAG, "WiFi Quality warning: " + String(quality) + "%");
            health_setStatus(HEALTH_WIFI_QUALITY, HEALTH_STATUS_WARNING);
        }
        else
        {
            health_setStatus(HEALTH_WIFI_QUALITY, HEALTH_STATUS_OK);
        }

        // Check Channel (some channels have interference issues)
        if (channel == 0)
        {
            Log::sys_error(PRIV_DAEMON_TAG, "WiFi Channel error: " + String(channel));
            health_setStatus(HEALTH_WIFI_CHANNEL, HEALTH_STATUS_ERROR);
        }
        else if (channel > 14) // 2.4GHz channels are 1-14
        {
            // 5GHz channels are fine
            health_setStatus(HEALTH_WIFI_CHANNEL, HEALTH_STATUS_OK);
        }
        else
        {
            health_setStatus(HEALTH_WIFI_CHANNEL, HEALTH_STATUS_OK);
        }

        // Check Ping latency (if available)
        if (pingMs > 0)
        {
            if (pingMs > 500)
            {
                Log::sys_error(PRIV_DAEMON_TAG, "WiFi Ping critical: " + String(pingMs) + " ms");
                health_setStatus(HEALTH_WIFI_PING, HEALTH_STATUS_ERROR);
            }
            else if (pingMs > 200)
            {
                Log::sys_warning(PRIV_DAEMON_TAG, "WiFi Ping warning: " + String(pingMs) + " ms");
                health_setStatus(HEALTH_WIFI_PING, HEALTH_STATUS_WARNING);
            }
            else
            {
                health_setStatus(HEALTH_WIFI_PING, HEALTH_STATUS_OK);
            }
        }
        else
        {
            health_setStatus(HEALTH_WIFI_PING, HEALTH_STATUS_UNKNOWN);
        }

        delay(1000 * HEALTH_WIFI_DAEMON); // Delay for the specified interval
    }
}

void HealthDaemons::startHealthWiFiDaemon(void)
{
    Log::sys_info(PRIV_DAEMON_TAG, "Starting health WiFi daemon...");

    xTaskCreatePinnedToCore(
        healthWiFiDaemonTask,
        "healthWiFiDaemon",
        8192,
        NULL,
        1,
        NULL,
        1);

    Log::sys_info(PRIV_DAEMON_TAG, "Health WiFi daemon started");
}