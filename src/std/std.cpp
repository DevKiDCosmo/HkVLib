#include "../include.h"

void delay(int ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

const char *TAG = "APP";

void serialprint(const String message)
{
    ESP_LOGI(TAG, "%s", message.c_str());
}