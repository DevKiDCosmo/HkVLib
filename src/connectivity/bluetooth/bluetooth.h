#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <vector>
#include <functional>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class BluetoothManager
{
public:
    using MessageCallback = std::function<void(const String &message, const String &senderMAC)>;
    using ConnectionCallback = std::function<void(const String &deviceMAC, bool connected)>;

    BluetoothManager();
    ~BluetoothManager();

    // Initialization
    bool begin(const String &deviceName);
    void end();

    // Server mode
    bool startServer();
    void stopServer();
    bool isServerRunning();

    // Client mode
    bool scanForDevices(uint32_t duration = 5);
    bool connectToDevice(const String &address);
    bool disconnectFromDevice();
    bool isConnected();

    // Data transfer
    bool sendMessage(const String &message);
    bool broadcastMessage(const String &message);

    // Device info
    String getDeviceName();
    String getMacAddress();
    int getRSSI();

    // Callbacks
    void onMessageReceived(MessageCallback callback);
    void onDeviceConnection(ConnectionCallback callback);

    // Connected devices
    std::vector<String> getConnectedDevices();
    int getConnectedDeviceCount();

    // Health metrics
    uint8_t getTxPower();
    bool isInitialized();

private:
    class ServerCallbacks;
    class CharacteristicCallbacks;

    String deviceName;
    String macAddress;
    bool initialized;
    bool serverRunning;
    bool clientConnected;

    BLEServer *pServer;
    BLECharacteristic *pCharacteristic;
    BLEClient *pClient;
    BLERemoteCharacteristic *pRemoteCharacteristic;

    MessageCallback messageCallback;
    ConnectionCallback connectionCallback;

    std::vector<String> connectedDevices;

    friend class ServerCallbacks;
    friend class CharacteristicCallbacks;
};

#endif // BLUETOOTH_H
