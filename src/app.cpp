#include "app.h"
#include "include.h"

const char *TAG = "APP";

#define LED_PIN 2

void App::app()
{
    ESP_LOGI(TAG, "App initialized");
    pinMode(LED_PIN, OUTPUT);
}

void App::update()
{
    // Main application logic goes here
    ESP_LOGI(TAG, "Main loop - Free heap: %d bytes", esp_get_free_heap_size());
    vTaskDelay(pdMS_TO_TICKS(10000)); // 10 second interval

    ESP_LOGI(TAG, "Doing some work in the main loop..., IP: %s", g_wifi->isConnected() ? g_wifi->getLocalIP().c_str() : "Not connected");

    digitalWrite(LED_PIN, !digitalRead(LED_PIN));

    // Using Makeblock-Libary for actual hardware interaction (e.g., sensors, actuators) would go here

    // As test. Try to disconnect to WiFi.
    if (g_wifi->isConnected())
    {
        ESP_LOGI(TAG, "Disconnecting WiFi to test reconnect logic...");
        g_wifi->disconnect();
    }
}