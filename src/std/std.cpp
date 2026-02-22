#include "std.h"
#include "../include.h"
#include "../config/config.h"
#include "../components/cyberpi/src/cyberpi.h"

void delay(int ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

const char *TAG = "APP";

void serialprint(const String message)
{
    ESP_LOGI(TAG, "%s", message.c_str());
}

void display_print(const String &message, DisplayPrintColor color)
{
    serialprint("[DISPLAY] display_print called: " + message);
    serialprint("[DISPLAY] cyber address: " + String((unsigned long)&cyber, HEX));
    if (!Display::is_ready())
    {
        serialprint("[DISPLAY] Display not ready!");
        return;
    }

    Display::draw_log(cyber, message, color);
    serialprint("[DISPLAY] draw_log completed");
}

void display_clear()
{
    serialprint("[DISPLAY] display_clear called");
    if (!Display::is_ready())
    {
        serialprint("[DISPLAY] Display not ready for clear!");
        return;
    }

    Display::clear(cyber);
    serialprint("[DISPLAY] clear completed");
}