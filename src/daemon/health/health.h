#pragma once

#include <stdint.h>

// Health Status Types
#define HEALTH_STATUS_OK 0
#define HEALTH_STATUS_WARNING 1
#define HEALTH_STATUS_ERROR 2
#define HEALTH_STATUS_CRITICAL 3
#define HEALTH_STATUS_UNKNOWN 4

// Health Check Types
#define HEALTH_WIFI_RSSI 0
#define HEALTH_WIFI_TXPOWER 1
#define HEALTH_WIFI_PING 2
#define HEALTH_WIFI_CHANNEL 3
#define HEALTH_WIFI_QUALITY 4
#define HEALTH_WIFI_LINKSPEED 5
#define HEALTH_WIFI_CONNECTED 6

// Global health status array (extern declaration)
extern uint8_t g_healthStatus[16];

// Health status functions
void health_setStatus(uint8_t healthType, uint8_t status);
uint8_t health_getStatus(uint8_t healthType);
const char *health_getStatusString(uint8_t status);

class HealthDaemons
{
public:
    static void startHealthWiFiDaemon(void);
    static void startHealthDaemons(void);
};