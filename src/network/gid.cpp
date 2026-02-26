#include "gid.h"
#include "../serial/log.h"
#include <Arduino.h>
#include "request.h"
#include "config/config.h"
#include "esp_wifi.h"

void GID::gID(void)
{
    // Get MAC Address
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);

    // Format MAC address as String (XX:XX:XX:XX:XX:XX)
    char macBuf[18];
    snprintf(macBuf, sizeof(macBuf), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    String macStr(macBuf);

    Log::sys_info("DHCP_ID", "Device MAC: " + macStr);

    // Store MAC address globally (safe: String owns its memory)
    MAC_ADDR = macStr;

    // Initialize HTTP client with config-aware server configuration
    HttpRequest httpClient;

    Log::sys_info("DHCP_ID", "Requesting device ID from server...");

    // Send ID request to SERVER:PORT/id/:mac/:teamid
    String endpoint = "/id/" + macStr + "/" + String(TEAMID);
    HttpResponse response = httpClient.get(endpoint);

    if (response.success && response.statusCode == 200)
    {
        // Parse device ID from response body
        DEVICE_ID = response.body.toInt();
        String logMsg = "Device ID assigned: " + String(DEVICE_ID) + " (MAC: " + macStr + ", Team: " + String(TEAMID) + ")";
        Log::sys_info("DHCP_ID", logMsg);
    }
    else
    {
        Log::sys_error("DHCP_ID", "Failed to get device ID - Status: " + String(response.statusCode));
        DEVICE_ID = -1; // Indicate failure
    }
}