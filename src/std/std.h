#pragma once

#include <Arduino.h>
#include "../display/display.h"

void delay(int ms);
void serialprint(const String message);

using DisplayPrintColor = Display::PresetColor;
void display_print(const String &message, DisplayPrintColor color = DisplayPrintColor::White);
void display_clear();