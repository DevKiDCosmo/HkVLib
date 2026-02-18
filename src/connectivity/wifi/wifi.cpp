#include "wifi.h"
#include "../../config/config.h"
#include "../../serial/log.h"

WiFiConnect::WiFiConnect() : connected(false) {}

bool WiFiConnect::connect(const String &ssid, const String &password, unsigned long timeout_ms)
{
    Log::sys_info("WIFI", "Connecting to WiFi: " + ssid);

    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < timeout_ms)
    {
        delay(500);
        Log::sys_infoflag("WIFI", ".", DEBUG_FLAG_EXTENSIVE);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        connected = true;
        Log::sys_info("WIFI", "Connected! IP: " + WiFi.localIP().toString());
        return true;
    }
    else
    {
        connected = false;
        Log::sys_info("WIFI", "Connection failed!");
        return false;
    }
}

bool WiFiConnect::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

void WiFiConnect::disconnect()
{
    if (!isConnected())
    {
        Log::sys_info("WIFI", "Already disconnected");
        return;
    }

    if (ONLINE_MANDATORY)
    {
        Log::sys_warning("WIFI", "Online mandatory is enabled, skipping disconnect!");
        return;
    }
    WiFi.disconnect();
    connected = false;
    Log::sys_info("WIFI", "Disconnected");
}

String WiFiConnect::getLocalIP()
{
    return WiFi.localIP().toString();
}

// WiFi Health Metrics Implementations

int8_t WiFiConnect::getRSSI()
{
    if (!isConnected())
        return -100;
    return WiFi.RSSI();
}

uint8_t WiFiConnect::getTxPower()
{
    return WiFi.getTxPower();
}

uint8_t WiFiConnect::getChannel()
{
    if (!isConnected())
        return 0;
    return WiFi.channel();
}

String WiFiConnect::getSSID()
{
    if (!isConnected())
        return "";
    return WiFi.SSID();
}

String WiFiConnect::getBSSID()
{
    if (!isConnected())
        return "";
    return WiFi.BSSIDstr();
}

uint8_t WiFiConnect::getEncryptionType()
{
    if (!isConnected())
        return 0;
    return WiFi.encryptionType(0);
}

int8_t WiFiConnect::getSignalQuality()
{
    if (!isConnected())
        return 0;
    int8_t rssi = WiFi.RSSI();
    // Convert RSSI to percentage (0-100%)
    // RSSI typically ranges from -100 dBm (worst) to -30 dBm (best)
    if (rssi <= -100)
        return 0;
    if (rssi >= -30)
        return 100;
    return 2 * (rssi + 100);
}

uint32_t WiFiConnect::getPingLatency()
{
    if (!isConnected())
        return 0;
    IPAddress gateway = WiFi.gatewayIP();
    if (gateway == IPAddress(0, 0, 0, 0))
        return 0;

    // Simple ping using Arduino ping library or WiFi ping
    // For ESP32, we can use the built-in ping functionality
    return 0; // Placeholder - implement with actual ping if needed
}

IPAddress WiFiConnect::getGatewayIP()
{
    if (!isConnected())
        return IPAddress(0, 0, 0, 0);
    return WiFi.gatewayIP();
}

IPAddress WiFiConnect::getSubnet()
{
    if (!isConnected())
        return IPAddress(0, 0, 0, 0);
    return WiFi.subnetMask();
}

String WiFiConnect::getMacAddress()
{
    return WiFi.macAddress();
}
