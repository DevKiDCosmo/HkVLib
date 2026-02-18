#include "health.h"
#include "../../serial/log.h"

static const char *HEALTH_TAG = "HEALTH";

// Global health status array
uint8_t g_healthStatus[16] = {
    HEALTH_STATUS_UNKNOWN, // HEALTH_WIFI_RSSI
    HEALTH_STATUS_UNKNOWN, // HEALTH_WIFI_TXPOWER
    HEALTH_STATUS_UNKNOWN, // HEALTH_WIFI_PING
    HEALTH_STATUS_UNKNOWN, // HEALTH_WIFI_CHANNEL
    HEALTH_STATUS_UNKNOWN, // HEALTH_WIFI_QUALITY
    HEALTH_STATUS_UNKNOWN, // HEALTH_WIFI_LINKSPEED
    HEALTH_STATUS_UNKNOWN  // HEALTH_WIFI_CONNECTED
};

void health_setStatus(uint8_t healthType, uint8_t status)
{
    if (healthType < 16)
    {
        uint8_t oldStatus = g_healthStatus[healthType];
        g_healthStatus[healthType] = status;

        if (oldStatus != status)
        {
            Log::sys_info(HEALTH_TAG, "Health [" + String(healthType) + "] changed: " + health_getStatusString(oldStatus) + " -> " + health_getStatusString(status));
        }
    }
}

uint8_t health_getStatus(uint8_t healthType)
{
    if (healthType < 16)
    {
        return g_healthStatus[healthType];
    }
    return HEALTH_STATUS_UNKNOWN;
}

const char *health_getStatusString(uint8_t status)
{
    switch (status)
    {
    case HEALTH_STATUS_OK:
        return "OK";
    case HEALTH_STATUS_WARNING:
        return "WARNING";
    case HEALTH_STATUS_ERROR:
        return "ERROR";
    case HEALTH_STATUS_CRITICAL:
        return "CRITICAL";
    default:
        return "UNKNOWN";
    }
}

void HealthDaemons::startHealthDaemons(void)
{
    // Start
    HealthDaemons::startHealthWiFiDaemon();
}