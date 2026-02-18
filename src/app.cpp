#include "app.h"
#include "include.h"
#include "std/std.h"

#define LED_PIN 2

void App::app()
{
    serialprint("App initialized");
    pinMode(LED_PIN, OUTPUT);
}

void App::update()
{
    // Main application logic goes here
    serialprint("Main loop - Free heap: " + String(esp_get_free_heap_size()) + " bytes");
    delay(10000);

    serialprint("Doing some work in the main loop..., IP: " + (g_wifi->isConnected() ? g_wifi->getLocalIP() : "Not connected"));

    digitalWrite(LED_PIN, !digitalRead(LED_PIN));

    // Using Makeblock-Libary for actual hardware interaction (e.g., sensors, actuators) would go here

    if (g_wifi->isConnected())
    {
        serialprint("Disconnecting WiFi to test reconnect logic...");
        g_wifi->disconnect();
    }
}