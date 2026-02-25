#include "app.h"
#include "include.h"
#include "std/std.h"

#define LED_PIN 2

void App::init()
{
    pinMode(LED_PIN, OUTPUT);
    serialprint("LED pin configured");
    display_clear();
}

void App::update()
{
    serialprint("=== App::update() START ===");
    serialprint("Main loop - Free heap: " + String(esp_get_free_heap_size()) + " bytes");
    delay(1000);

    serialprint("Doing some work in the main loop..., IP: " + (g_wifi->isConnected() ? g_wifi->getLocalIP() : "Not connected"));

    digitalWrite(LED_PIN, !digitalRead(LED_PIN));

    // Using Makeblock-Libary for actual hardware interaction (e.g., sensors, actuators) would go here

    // TODO: Bug. Hello cannot be rendered
    // Every function scope cannot render text except for init. Suspect some kind of memory corruption or display state issue after init() completes. Need to investigate further.
    display_print("Hello.", DisplayPrintColor::Green);
    display_print("Without color");
    delay(1000);
}