#include "health.h"
#include "../../include.h"
#include "../../connectivity/wifi/wifi.h"

static const char *PRIV_DAEMON_TAG = "HealthWiFi";

void healthWiFiDaemonTask(void *pvParameters)
{
    ESP_LOGI(PRIV_DAEMON_TAG, "Health WiFi daemon started on Core %d", xPortGetCoreID());

    while (true)
    {
        // Check if WiFi is connected
        if (!g_wifi->isConnected())
        {
            ESP_LOGW(PRIV_DAEMON_TAG, "WiFi not connected");
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

        ESP_LOGI(PRIV_DAEMON_TAG, "WiFi Status - SSID: %s, IP: %s, Gateway: %s",
                 ssid.c_str(), localIP.c_str(), gateway.toString().c_str());
        ESP_LOGI(PRIV_DAEMON_TAG, "WiFi Metrics - RSSI: %d dBm, Quality: %d%%, Channel: %u",
                 rssi, quality, channel);
        ESP_LOGI(PRIV_DAEMON_TAG, "WiFi Metrics - TX Power: %u dBm, BSSID: %s",
                 txPower, bssid.c_str());

        // Check RSSI (signal strength)
        if (rssi < -80)
        {
            ESP_LOGE(PRIV_DAEMON_TAG, "WiFi RSSI critical: %d dBm", rssi);
            health_setStatus(HEALTH_WIFI_RSSI, HEALTH_STATUS_ERROR);
        }
        else if (rssi < -70)
        {
            ESP_LOGW(PRIV_DAEMON_TAG, "WiFi RSSI warning: %d dBm", rssi);
            health_setStatus(HEALTH_WIFI_RSSI, HEALTH_STATUS_WARNING);
        }
        else
        {
            health_setStatus(HEALTH_WIFI_RSSI, HEALTH_STATUS_OK);
        }

        // Check TX Power
        if (txPower < 10)
        {
            ESP_LOGE(PRIV_DAEMON_TAG, "WiFi TX Power critical: %u dBm", txPower);
            health_setStatus(HEALTH_WIFI_TXPOWER, HEALTH_STATUS_ERROR);
        }
        else if (txPower < 15)
        {
            ESP_LOGW(PRIV_DAEMON_TAG, "WiFi TX Power warning: %u dBm", txPower);
            health_setStatus(HEALTH_WIFI_TXPOWER, HEALTH_STATUS_WARNING);
        }
        else
        {
            health_setStatus(HEALTH_WIFI_TXPOWER, HEALTH_STATUS_OK);
        }

        // Check Signal Quality
        if (quality < 30)
        {
            ESP_LOGE(PRIV_DAEMON_TAG, "WiFi Quality critical: %d%%", quality);
            health_setStatus(HEALTH_WIFI_QUALITY, HEALTH_STATUS_ERROR);
        }
        else if (quality < 50)
        {
            ESP_LOGW(PRIV_DAEMON_TAG, "WiFi Quality warning: %d%%", quality);
            health_setStatus(HEALTH_WIFI_QUALITY, HEALTH_STATUS_WARNING);
        }
        else
        {
            health_setStatus(HEALTH_WIFI_QUALITY, HEALTH_STATUS_OK);
        }

        // Check Channel (some channels have interference issues)
        if (channel == 0)
        {
            ESP_LOGE(PRIV_DAEMON_TAG, "WiFi Channel error: %u", channel);
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
                ESP_LOGE(PRIV_DAEMON_TAG, "WiFi Ping critical: %u ms", pingMs);
                health_setStatus(HEALTH_WIFI_PING, HEALTH_STATUS_ERROR);
            }
            else if (pingMs > 200)
            {
                ESP_LOGW(PRIV_DAEMON_TAG, "WiFi Ping warning: %u ms", pingMs);
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
    ESP_LOGI(PRIV_DAEMON_TAG, "Starting health WiFi daemon...");

    xTaskCreatePinnedToCore(
        healthWiFiDaemonTask,
        "healthWiFiDaemon",
        8192,
        NULL,
        1,
        NULL,
        1);

    ESP_LOGI(PRIV_DAEMON_TAG, "Health WiFi daemon started");
}