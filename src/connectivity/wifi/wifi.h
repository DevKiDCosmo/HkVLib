#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

#include <Arduino.h>
#include <WiFi.h>

class WiFiConnect {
public:
    WiFiConnect();
    bool connect(const char* ssid, const char* password, unsigned long timeout_ms = 10000);
    bool isConnected();
    void disconnect();
    String getLocalIP();
    
private:
    bool connected;
};

#endif // WIFI_CONNECT_H
