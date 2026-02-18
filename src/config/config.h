
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// WLAN configuration
#define WLAN_SSID "Vodafone-9A6C"
#define WLAN_PASSWORD "nFxDLFAv4jYpDbgt"
#define ONLINE_MANDATORY true
#define ONLINE_LOCK false // If ofline, lock the device to prevent any operation until online again (use with ONLINE_MANDATORY)
#define WLAN_PRIORITY 20  // in s interval for tries. Std: 5

#define BACKUP_WLAN_SSID "NAMANAMANAMANAN"
#define BACKUP_WLAN_PASSWORD "Namnamnam"

// Server configuration
#define SERVER "192.168.0.171"
#define PORT 8080
#define HEARTBEAT_SERVER_AVAIBILITY 30 // in s
#define HEARTBEAT_DEVICE_AVAIBILITY 10 // in s

// Daemon Interval Configuration
#define HEALTH_WIFI_DAEMON 10 // in s

// Firmware
#define BUILD "12203984"
#define DATE "02/18/2026"
#define GUID ""
#define VERSION "v1.0"

// DEBUG Config
#define DEBUG_FLAG_EXTENSIVE false
#define DEBUG_ERROR true
#define DEBUG_NORMAL true
#define DEBUG_SERIAL true
#define DEBUG_WARNING true

// TEAM Configuration and Name
#define TEAMID 0
#define TEAM_NAME ""
#define DEVICE_NAME ""

// Forward declaration (full definition in connectivity/wifi/wifi.h)
class WiFiConnect;

// GLOBAL Variables (defined in main.cpp)
extern int DEVICE_ID;
extern String MAC_ADDR;

extern WiFiConnect *g_wifi;
extern String g_ssid;
extern String g_password;

#endif // CONFIG_H