#include "wifi.h"
#include "esp_log.h"

WiFiConnect::WiFiConnect() : connected(false) {}

bool WiFiConnect::connect(const char* ssid, const char* password, unsigned long timeout_ms) {
    ESP_LOGI("WIFI", "Connecting to WiFi: %s", ssid);
    
    WiFi.begin(ssid, password);
    
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < timeout_ms) {
        delay(500);
        ESP_LOGI("WIFI", ".");
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
    WiFi.disconnect();
    connected = false;
    ESP_LOGI("WIFI", "Disconnected");
}

String WiFiConnect::getLocalIP() {
    return WiFi.localIP().toString();
}
