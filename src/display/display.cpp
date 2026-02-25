#include "display.h"
#include "boot_bitmap.h"
#include "team_bitmap.h"
#include "../serial/log.h"

#include <algorithm>

struct LogMessage
{
    String text;
    uint16_t color;
};

static std::vector<LogMessage> logMessages;
static bool g_displayReady = false;
static std::vector<Bitmap *> tempBitmaps; // Store bitmaps until after rendering

namespace
{
    constexpr uint8_t kLcdSize = 128;

    const char *TAG = "DISPLAY";

    std::vector<wchar_t> to_wide(const String &text)
    {
        std::vector<wchar_t> wide;
        wide.reserve(text.length() + 1);
        for (size_t i = 0; i < text.length(); ++i)
        {
            wide.push_back(static_cast<wchar_t>(static_cast<uint8_t>(text[i])));
        }
        wide.push_back(L'\0');
        return wide;
    }

    void destroy_bitmap(Bitmap *bitmap)
    {
        if (bitmap == nullptr)
        {
            return;
        }
        if (bitmap->buffer != nullptr)
        {
            free(bitmap->buffer);
            bitmap->buffer = nullptr;
        }
        delete bitmap;
    }

    void draw_text(CyberPi &cyber, uint8_t x, uint8_t y, const String &text, uint8_t fontSize, uint16_t color)
    {
        if (text.length() == 0)
        {
            return;
        }

        std::vector<wchar_t> wide = to_wide(text);
        Bitmap *textBitmap = cyber.create_text(wide.data(), color, fontSize);
        if (textBitmap == nullptr)
        {
            return;
        }

        cyber.set_bitmap(x, y, textBitmap);
        // Store bitmap for later cleanup - DON'T destroy before rendering!
        tempBitmaps.push_back(textBitmap);
    }

    void cleanup_bitmaps()
    {
        for (Bitmap *bmp : tempBitmaps)
        {
            destroy_bitmap(bmp);
        }
        tempBitmaps.clear();
    }
}

Display::Display() : width(kLcdSize), height(kLcdSize), title("HkVLib")
{
}

Display::~Display()
{
}

void Display::draw_boot(CyberPi &cyber)
{
    g_displayReady = true;

    const uint16_t white = color(cyber, PresetColor::White);
    const uint16_t accent = color(cyber, PresetColor::Cyan);

    cyber.clean_lcd();
    draw_bitmap(cyber, 0, 0, boot_bmp_width, boot_bmp_height, boot_bmp_pixels);
    cyber.render_lcd();
    delay(2000);

    cyber.clean_lcd();

    for (uint8_t x = 8; x < kLcdSize - 8; ++x)
    {
        cyber.set_lcd_pixel(x, 22, accent);
        cyber.set_lcd_pixel(x, 23, accent);
        cyber.set_lcd_pixel(x, 104, accent);
        cyber.set_lcd_pixel(x, 105, accent);
    }
    for (uint8_t y = 22; y <= 105; ++y)
    {
        cyber.set_lcd_pixel(8, y, accent);
        cyber.set_lcd_pixel(9, y, accent);
        cyber.set_lcd_pixel(kLcdSize - 10, y, accent);
        cyber.set_lcd_pixel(kLcdSize - 9, y, accent);
    }

    draw_text(cyber, 16, 40, "HkVLib", 18, white);
    draw_text(cyber, 16, 68, "Firmware Boot", 12, white);

    cyber.render_lcd();
    cleanup_bitmaps();
}

void Display::draw_team(CyberPi &cyber)
{
    if (!g_displayReady)
    {
        return;
    }

    cleanup_bitmaps(); // Clean up any previous bitmaps
    cyber.clean_lcd();
    draw_bitmap(cyber, 0, 0, team_width, team_height, team_pixels);
    cyber.render_lcd();
    delay(25); // Wait for LCD rendering throttle
}

void Display::draw_log(CyberPi &cyber, const String &message, PresetColor preset)
{
    Log::sys_infoflag(TAG, "draw_log called: " + message, DEBUG_DISPLAY);
    Log::sys_info(TAG, "cyber address in draw_log: " + String((unsigned long)&cyber, HEX));
    if (!g_displayReady)
    {
        Log::sys_warning(TAG, "Display not ready in draw_log!");
        return;
    }

    constexpr uint8_t fontSize = 12;
    constexpr size_t maxCharPerLine = 20;
    constexpr size_t maxLines = 7;
    const uint16_t textColor = color(cyber, preset);

    std::vector<String> wrapped;
    wrapped.reserve((message.length() / maxCharPerLine) + 2);

    String remaining = message;
    while (remaining.length() > 0)
    {
        int newlinePos = remaining.indexOf('\n');
        String segment;
        if (newlinePos >= 0)
        {
            segment = remaining.substring(0, newlinePos);
            remaining = remaining.substring(newlinePos + 1);
        }
        else
        {
            segment = remaining;
            remaining = "";
        }

        if (segment.length() == 0)
        {
            wrapped.push_back("");
            continue;
        }

        while (segment.length() > maxCharPerLine)
        {
            wrapped.push_back(segment.substring(0, maxCharPerLine));
            segment = segment.substring(maxCharPerLine);
        }
        wrapped.push_back(segment);
    }

    if (wrapped.empty())
    {
        wrapped.push_back("");
    }

    for (const String &line : wrapped)
    {
        logMessages.push_back({line, textColor});
    }

    if (logMessages.size() > maxLines)
    {
        logMessages.erase(logMessages.begin(), logMessages.begin() + (logMessages.size() - maxLines));
    }

    cleanup_bitmaps(); // Clean up any previous bitmaps
    cyber.clean_lcd();
    Log::sys_infoflag(TAG, "LCD cleaned, drawing " + String(logMessages.size()) + " messages", DEBUG_DISPLAY);
    for (size_t lineIndex = 0; lineIndex < logMessages.size(); ++lineIndex)
    {
        const uint8_t y = static_cast<uint8_t>(4 + lineIndex * (fontSize + 4));
        Log::sys_infoflag(TAG, "Drawing line " + String(lineIndex) + ": " + logMessages[lineIndex].text, DEBUG_DISPLAY);
        draw_text(cyber, 2, y, logMessages[lineIndex].text, fontSize, logMessages[lineIndex].color);
        Log::sys_infoflag(TAG, "Line " + String(lineIndex) + " drawn successfully", DEBUG_DISPLAY);
    }
    Log::sys_infoflag(TAG, "About to call cyber.render_lcd()...", DEBUG_DISPLAY);
    cyber.render_lcd();
    delay(25); // Wait for LCD rendering throttle (20ms minimum)
    Log::sys_infoflag(TAG, "LCD rendered successfully!", DEBUG_DISPLAY);
    cleanup_bitmaps(); // NOW destroy the bitmaps after rendering
}

uint16_t Display::color(CyberPi &cyber, PresetColor preset)
{
    uint32_t rgb = 0xFFFFFF;
    switch (preset)
    {
    case PresetColor::Black:
        rgb = 0x000000;
        break;
    case PresetColor::White:
        rgb = 0xFFFFFF;
        break;
    case PresetColor::Red:
        rgb = 0xFF3B30;
        break;
    case PresetColor::Green:
        rgb = 0x34C759;
        break;
    case PresetColor::Blue:
        rgb = 0x007AFF;
        break;
    case PresetColor::Yellow:
        rgb = 0xFFCC00;
        break;
    case PresetColor::Cyan:
        rgb = 0x32ADE6;
        break;
    case PresetColor::Magenta:
        rgb = 0xAF52DE;
        break;
    case PresetColor::Orange:
        rgb = 0xFF9500;
        break;
    case PresetColor::Gray:
        rgb = 0x8E8E93;
        break;
    }

    return cyber.color24_to_16(rgb);
}

void Display::draw_bitmap(CyberPi &cyber, uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint16_t *pixels)
{
    if (pixels == nullptr || width == 0 || height == 0)
    {
        return;
    }

    Bitmap bitmap = {};
    bitmap.width = width;
    bitmap.height = height;
    bitmap.buffer = const_cast<uint16_t *>(pixels);
    cyber.set_bitmap(x, y, &bitmap);
}

void Display::initialize()
{
    g_displayReady = true;
}

bool Display::is_ready()
{
    return g_displayReady;
}

void Display::render()
{
}

void Display::clear(CyberPi &cyber)
{
    Log::sys_infoflag(TAG, "Display::clear called", DEBUG_DISPLAY);
    logMessages.clear();
    cleanup_bitmaps(); // Clean up any bitmaps
    // Reset to a blank screen
    if (g_displayReady)
    {
        cyber.clean_lcd();
        cyber.render_lcd();
        delay(25); // Wait for LCD rendering throttle
        Log::sys_infoflag(TAG, "Display cleared and rendered", DEBUG_DISPLAY);
    }
    else
    {
        Log::sys_warning(TAG, "Display not ready in clear!");
    }
}

void Display::update()
{
}

void Display::shutdown()
{
    g_displayReady = false;
}

void Display::setTitle(const std::string &newTitle)
{
    title = newTitle;
}

void Display::setSize(int newWidth, int newHeight)
{
    width = std::max(1, newWidth);
    height = std::max(1, newHeight);
}