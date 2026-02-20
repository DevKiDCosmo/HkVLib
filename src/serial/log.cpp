#include "log.h"
#include "../display/display.h"
#include "../config/config.h"

void Log::sys_infoflag(const char *Tag, String msg, bool Flag)
{
    if (Flag)
        ESP_LOGI(Tag, "%s", msg.c_str());
}

void Log::sys_info(const char *Tag, String msg)
{
    if (DEBUG_NORMAL)
        ESP_LOGI(Tag, "%s", msg.c_str());
}

void Log::sys_warning(const char *Tag, String msg)
{
    if (DEBUG_WARNING)
        ESP_LOGW(Tag, "%s", msg.c_str());
}

void Log::sys_error(const char *Tag, String msg)
{
    if (DEBUG_ERROR)
        ESP_LOGE(Tag, "%s", msg.c_str());
}

void Log::sys_debug(const char *Tag, String msg)
{
    if (DEBUG_VERBOSE)
        ESP_LOGD(Tag, "%s", msg.c_str());
}