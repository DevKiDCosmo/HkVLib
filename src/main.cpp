#include "esp_log.h"
#include <Arduino.h>
#include "connectivity/wifi/wifi.h"
#include "network/request.h"
#include "config/config.h"
#include "esp_wifi.h"

#include "daemon/daemon.h"
#include "network/gid.h"
#include "app.h"

static const char *TAG = "MAIN";
static const char *NET_TAG = "NET_DAEMON";

// Global WiFi instance shared between main and daemon
WiFiConnect *g_wifi = nullptr;
String g_ssid;
String g_password;

// Global device configuration variables
int DEVICE_ID = 0;
String MAC_ADDR;

// ESP-IDF native serial setup (before Arduino initialization)
static void setup_serial(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    esp_log_level_set(NET_TAG, ESP_LOG_INFO);
}

void init_app(void)
{
    // ESP-IDF native logging (no Arduino dependency)
    setup_serial();
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "HkVLib Firmware Starting");
    ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "=================================");

    // Initialize Arduino framework
    initArduino();
    ESP_LOGI(TAG, "Arduino framework initialized");

    // Initialize Serial for reading commands
    Serial.begin(115200);
    ESP_LOGI(TAG, "Serial initialized at 115200 baud");

    // Initialize WiFi using Arduino library
    ESP_LOGI(TAG, "Initializing WiFi...");
    g_wifi = new WiFiConnect();
    g_ssid = WLAN_SSID;
    g_password = WLAN_PASSWORD;

    if (g_wifi->connect(g_ssid, g_password))
    {
        ESP_LOGI(TAG, "WiFi connected successfully");
        ESP_LOGI(TAG, "IP Address: %s", g_wifi->getLocalIP().c_str());
    }
    else
    {
        ESP_LOGE(TAG, "WiFi connection failed! Attempting backup credentials...");
        g_ssid = BACKUP_WLAN_SSID;
        g_password = BACKUP_WLAN_PASSWORD;
        if (g_wifi->connect(g_ssid, g_password))
        {
            ESP_LOGI(TAG, "Backup WiFi connected successfully");
            ESP_LOGI(TAG, "IP Address: %s", g_wifi->getLocalIP().c_str());
        }
        else
        {
            ESP_LOGE(TAG, "WiFi connection failed! Daemon will retry... with old credentials");
            g_ssid = WLAN_SSID;
            g_password = WLAN_PASSWORD;
        }
    }

    ESP_LOGI(TAG, "Setup complete. Starting background services...");

    // Start network daemon after WiFi init
    ESP_LOGI(TAG, "Starting daemons...");

    Daemon::startNetworkDaemon();
    Daemon::startHeartbeatDaemon();

    // Start serial input daemon for immediate command processing.
    Daemon::startSerialInputDaemon();

    // Starting DHCP ID Client Configuration
    GID::gID();
}

extern "C" void app_main(void)
{
    init_app();
    App::app();
    ESP_LOGI(TAG, "Main loop starting on Core %d", xPortGetCoreID());
    while (true)
    {
        App::update();
    }
}