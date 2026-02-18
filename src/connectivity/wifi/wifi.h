#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

#include <Arduino.h>
#include <WiFi.h>

class WiFiConnect
{
public:
    WiFiConnect();
    bool connect(const String &ssid, const String &password, unsigned long timeout_ms = 10000);
    bool isConnected();
    void disconnect();
    String getLocalIP();

    // WiFi Health Metrics
    int8_t getRSSI();            // Signal strength in dBm
    uint8_t getTxPower();        // Transmit power in dBm
    uint8_t getChannel();        // WiFi channel
    String getSSID();            // Network name
    String getBSSID();           // Access point MAC address
    uint8_t getEncryptionType(); // Security type
    int8_t getSignalQuality();   // Signal quality (0-100%)
    uint32_t getPingLatency();   // Ping to gateway in ms
    IPAddress getGatewayIP();    // Gateway IP
    IPAddress getSubnet();       // Subnet mask
    String getMacAddress();      // Device MAC address

private:
    bool connected;
};

#endif // WIFI_CONNECT_H
