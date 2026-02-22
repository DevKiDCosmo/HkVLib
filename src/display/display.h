#pragma once

#include <Arduino.h>
#include <vector>
#include "../components/cyberpi/src/cyberpi.h"

class Display
{
public:
    enum class PresetColor : uint8_t
    {
        Black,
        White,
        Red,
        Green,
        Blue,
        Yellow,
        Cyan,
        Magenta,
        Orange,
        Gray
    };

    Display();
    ~Display();

    static void draw_boot(CyberPi &cyber);
    static void draw_team(CyberPi &cyber);
    static void draw_log(CyberPi &cyber, const String &message, PresetColor preset = PresetColor::White);
    static uint16_t color(CyberPi &cyber, PresetColor preset);
    static void draw_bitmap(CyberPi &cyber, uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint16_t *pixels);
    static void initialize();
    static bool is_ready();
    static void render();
    static void clear(CyberPi &cyber);
    static void update();
    static void shutdown();

    void setTitle(const std::string &title);
    void setSize(int width, int height);

private:
    int width;
    int height;
    std::string title;
};