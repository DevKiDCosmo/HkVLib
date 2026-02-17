#include "esp_log.h"
#include <Arduino.h>
#include "connectivity/wifi/wifi.h"
#include "network/request.h"

static const char *TAG = "MAIN";
static const char *NET_TAG = "NET_DAEMON";

#define SERVER "192.168.0.70" 
#define PORT 8080

// Global WiFi instance shared between main and daemon
WiFiConnect *g_wifi = nullptr;
const char *g_ssid = nullptr;
const char *g_password = nullptr;

// ESP-IDF native serial setup (before Arduino initialization)
static void setup_serial(void) {
    esp_log_level_set(TAG, ESP_LOG_INFO);
    esp_log_level_set(NET_TAG, ESP_LOG_INFO);
}

// Network daemon task - runs asynchronously on Core 0
void networkDaemonTask(void *pvParameters) {
    ESP_LOGI(NET_TAG, "Network daemon started on Core %d", xPortGetCoreID());
    
    // Wait a moment for initialization to complete
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    while (true) {
        if (g_wifi != nullptr && g_ssid != nullptr && g_password != nullptr) {
            // Check WiFi status periodically
            if (!g_wifi->isConnected()) {
                ESP_LOGW(NET_TAG, "WiFi disconnected! Attempting reconnect...");
                if (g_wifi->connect(g_ssid, g_password)) {
                    ESP_LOGI(NET_TAG, "WiFi reconnected. IP: %s", g_wifi->getLocalIP().c_str());
                } else {
                    ESP_LOGE(NET_TAG, "WiFi reconnect failed!");
                }
            } else {
                // WiFi is connected - just log status occasionally
                static int counter = 0;
                if (++counter % 12 == 0) {  // Every 60 seconds
                    ESP_LOGI(NET_TAG, "WiFi OK - IP: %s", g_wifi->getLocalIP().c_str());
                    counter = 0;
                }
            }
        }
        
        // Check every 5 seconds
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void startNetworkDaemon(void) {
    ESP_LOGI(TAG, "Starting network daemon...");
    
    // Create network daemon task on Core 0
    // Stack size: 4096 bytes, Priority: 1 (low), Core: 0
    xTaskCreatePinnedToCore(
        networkDaemonTask,      // Task function
        "NetworkDaemon",        // Task name
        4096,                   // Stack size
        NULL,                   // Parameter
        1,                      // Priority
        NULL,                   // Task handle
        0                       // Core 0
    );
    
    ESP_LOGI(TAG, "Network daemon started");
}

void heartbeatDaemonTask(void *pvParameters) {
    ESP_LOGI("HEARTBEAT", "Heartbeat daemon started on Core %d", xPortGetCoreID());
    
    // Initialize HTTP client with server configuration
    HttpRequest httpClient(SERVER, PORT);
    
    while (true) {
        ESP_LOGI("HEARTBEAT", "System is alive - Free heap: %d bytes", esp_get_free_heap_size());
        
        // Send heartbeat request to SERVER:PORT/heartbeat
        HttpResponse response = httpClient.get("/heartbeat");
        if (response.success) {
            ESP_LOGI("HEARTBEAT", "Heartbeat sent successfully - Status: %d, Response: %s", 
                     response.statusCode, response.body.c_str());
        } else {
            ESP_LOGE("HEARTBEAT", "Heartbeat failed - Status: %d", response.statusCode);
        }
        
        vTaskDelay(pdMS_TO_TICKS(30000));  // Every 30 seconds
    }
}

void startHeartbeatDaemon(void) {
    ESP_LOGI(TAG, "Starting heartbeat daemon...");
    
    // Create a simple heartbeat task on Core 1
    xTaskCreatePinnedToCore(
        heartbeatDaemonTask,
        "HeartbeatDaemon",
        8192,
        NULL,
        1,
        NULL,
        1
    );
    
    ESP_LOGI(TAG, "Heartbeat daemon started");
}

void init_app(void) {
    // ESP-IDF native logging (no Arduino dependency)
    setup_serial();
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "HkVLib Firmware Starting");
    ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "=================================");

    // Initialize Arduino framework
    initArduino();
    ESP_LOGI(TAG, "Arduino framework initialized");

    // Initialize WiFi using Arduino library
    ESP_LOGI(TAG, "Initializing WiFi...");
    g_wifi = new WiFiConnect();
    g_ssid = "Vodafone-9A6C";
    g_password = "nFxDLFAv4jYpDbgt";

    if (g_wifi->connect(g_ssid, g_password)) {
        ESP_LOGI(TAG, "WiFi connected successfully");
        ESP_LOGI(TAG, "IP Address: %s", g_wifi->getLocalIP().c_str());
    } else {
        ESP_LOGE(TAG, "WiFi connection failed! Daemon will retry...");
    }

    ESP_LOGI(TAG, "Setup complete. Starting background services...");
    
    // Start network daemon after WiFi init
    startNetworkDaemon();
    startHeartbeatDaemon();
}

extern "C" void app_main(void) {
    init_app();

    // Main loop runs independently on Core 1
    ESP_LOGI(TAG, "Main loop starting on Core %d", xPortGetCoreID());
    while (true) {

        // Main application logic goes here
        ESP_LOGI(TAG, "Main loop - Free heap: %d bytes", esp_get_free_heap_size());
        vTaskDelay(pdMS_TO_TICKS(10000));  // 10 second interval

        ESP_LOGI(TAG, "Doing some work in the main loop..., IP: %s", g_wifi->isConnected() ? g_wifi->getLocalIP().c_str() : "Not connected");
    }
}