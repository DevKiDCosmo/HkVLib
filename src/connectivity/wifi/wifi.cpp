#include "wifi.h"
#include "esp_log.h"
#include "../../config/config.h"

WiFiConnect::WiFiConnect() : connected(false) {}

bool WiFiConnect::connect(const String& ssid, const String& password, unsigned long timeout_ms) {
    ESP_LOGI("WIFI", "Connecting to WiFi: %s", ssid.c_str());
    
    WiFi.begin(ssid.c_str(), password.c_str());
    
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < timeout_ms) {
        delay(500);
        if (DEBUG_FLAG) {
            ESP_LOGI("WIFI", ".");
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        connected = true;
        ESP_LOGI("WIFI", "Connected! IP: %s", WiFi.localIP().toString().c_str());
        return true;
    } else {
        connected = false;
        ESP_LOGI("WIFI", "Connection failed!");
        return false;
    }
}

bool WiFiConnect::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiConnect::disconnect() {
    if (!isConnected()) {
        ESP_LOGI("WIFI", "Already disconnected");
        return;
    }

    if (ONLINE_MANDATORY) {
         ESP_LOGW("WIFI", "Online mandatory is enabled, skipping disconnect!");
        return;
    }
    WiFi.disconnect();
    connected = false;
    ESP_LOGI("WIFI", "Disconnected");
}

String WiFiConnect::getLocalIP() {
    return WiFi.localIP().toString();
}
