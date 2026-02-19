#include "wifi.h"
#include "../../config/config.h"
#include "../../serial/log.h"
#include "../../utility/utility.h"
#include "../../utility/init.h"

WiFiConnect::WiFiConnect() : connected(false), device_name(""), last_ssid(""), last_password(""), last_timeout_ms(10000) {}
static String Hash_WIFI;
static String Hash_WIFI_BACKUP;

void WiFiConnect::setDeviceName(const String &device_name)
{
    String effective_name = device_name;

    if (device_name.isEmpty())
    {
        Log::sys_warning("WIFI", "Device name is empty, using default hostname");
        Log::sys_warning("WIFI", "Default hostname will be used: " + String(WiFi.getHostname()));
        effective_name = String(WiFi.getHostname());
    }

    if (effective_name.length() > 32)
    {
        Log::sys_warning("WIFI", "Device name too long, truncating to 32 characters");
        effective_name = effective_name.substring(0, 32);
    }
    Log::sys_info("WIFI", "Setting device hostname: " + effective_name);
    this->device_name = effective_name;

    // ESP32 hostname is applied when starting a station connection. If already
    // connected, reconnect so the new hostname is effective immediately.
    if (isConnected())
    {
        if (last_ssid.isEmpty())
        {
            Log::sys_warning("WIFI", "Hostname updated, but no stored WiFi credentials available for reconnect");
            return;
        }

        Log::sys_info("WIFI", "Reconnecting WiFi to apply new hostname");
        WiFi.disconnect(false, false);
        WiFi.setHostname(effective_name.c_str());
        connected = false;

        if (!connect(last_ssid, last_password, last_timeout_ms))
        {
            Log::sys_error("WIFI", "Failed to reconnect after hostname change");
        }
    }
}

bool WiFiConnect::connect(const String &ssid, const String &password, unsigned long timeout_ms)
{
    if (WLAN_LOCK)
    {
        String current_hash = Utility::wlan_lock(ssid, password);

        if (Hash_WIFI.isEmpty() && !Init::value())
        {
            Hash_WIFI = current_hash;
        }
        else if (Hash_WIFI == current_hash)
        {
            Log::sys_info("WIFI", "WLAN Lock hash matches primary credentials");
        }
        else if (Hash_WIFI_BACKUP.isEmpty() && !Init::value())
        {
            // allow one backup credential set
            Hash_WIFI_BACKUP = current_hash;
        }
        else if (Hash_WIFI_BACKUP == current_hash)
        {
            Log::sys_info("WIFI", "WLAN Lock hash matches backup credentials");
        }
        else
        {
            Log::sys_error("WIFI", "WLAN Lock hash mismatch!");
            return false;
        }
    }

    last_ssid = ssid;
    last_password = password;
    last_timeout_ms = timeout_ms;

    Log::sys_info("WIFI", "Connecting to WiFi: " + ssid);

    if (!device_name.isEmpty())
    {
        WiFi.mode(WIFI_STA);
        if (WiFi.setHostname(device_name.c_str()))
        {
            Log::sys_info("WIFI", "Device hostname set: " + device_name);
        }
        else
        {
            Log::sys_warning("WIFI", "Failed to set device hostname: " + device_name);
        }
    }

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
