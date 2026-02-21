#include "bluetooth.h"
#include "../../serial/log.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

static const char *TAG = "BLUETOOTH";

class BluetoothManager::ServerCallbacks : public BLEServerCallbacks
{
public:
    explicit ServerCallbacks(BluetoothManager *manager) : pManager(manager) {}

    void onConnect(BLEServer *pServer) override
    {
        Log::sys_info(TAG, "Client connected");
        if (pManager->connectionCallback)
        {
            pManager->connectionCallback("unknown", true);
        }
    }

    void onDisconnect(BLEServer *pServer) override
    {
        Log::sys_info(TAG, "Client disconnected");
        if (pManager->connectionCallback)
        {
            pManager->connectionCallback("unknown", false);
        }
        pServer->getAdvertising()->start();
    }

private:
    BluetoothManager *pManager;
};

class BluetoothManager::CharacteristicCallbacks : public BLECharacteristicCallbacks
{
public:
    explicit CharacteristicCallbacks(BluetoothManager *manager) : pManager(manager) {}

    void onWrite(BLECharacteristic *pCharacteristic) override
    {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0)
        {
            String message = String(value.c_str());
            Log::sys_info(TAG, "Received message: " + message);

            if (pManager->messageCallback)
            {
                pManager->messageCallback(message, "server");
            }
        }
    }

    void onRead(BLECharacteristic *pCharacteristic) override
    {
        Log::sys_info(TAG, "Characteristic read");
    }

private:
    BluetoothManager *pManager;
};

bool BluetoothManager::startServer()
{
    if (!initialized)
    {
        Log::sys_error(TAG, "Bluetooth not initialized");
        return false;
    }

    if (serverRunning)
    {
        Log::sys_warning(TAG, "Server already running");
        return true;
    }

    pServer = BLEDevice::createServer();
    if (!pServer)
    {
        Log::sys_error(TAG, "Failed to create BLE server");
        return false;
    }

    pServer->setCallbacks(new ServerCallbacks(this));

    BLEService *pService = pServer->createService(SERVICE_UUID);
    if (!pService)
    {
        Log::sys_error(TAG, "Failed to create BLE service");
        return false;
    }

    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_INDICATE);

    if (!pCharacteristic)
    {
        Log::sys_error(TAG, "Failed to create BLE characteristic");
        return false;
    }

    pCharacteristic->setCallbacks(new CharacteristicCallbacks(this));
    pCharacteristic->addDescriptor(new BLE2902());

    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    if (!pAdvertising)
    {
        Log::sys_error(TAG, "Failed to get BLE advertising");
        return false;
    }

    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x06);
    BLEDevice::startAdvertising();

    serverRunning = true;
    Log::sys_info(TAG, "BLE Server started");
    return true;
}

void BluetoothManager::stopServer()
{
    if (serverRunning)
    {
        if (pServer)
        {
            pServer->getAdvertising()->stop();
        }
        serverRunning = false;
        Log::sys_info(TAG, "BLE Server stopped");
    }
}

bool BluetoothManager::isServerRunning()
{
    return serverRunning;
}

bool BluetoothManager::sendMessage(const String &message)
{
    if (serverRunning && pCharacteristic)
    {
        pCharacteristic->setValue(message.c_str());
        pCharacteristic->notify();
        Log::sys_info(TAG, "Sent message (server): " + message);
        return true;
    }
    else if (clientConnected && pRemoteCharacteristic)
    {
        pRemoteCharacteristic->writeValue(message.c_str(), message.length());
        Log::sys_info(TAG, "Sent message (client): " + message);
        return true;
    }
    else
    {
        Log::sys_warning(TAG, "Cannot send message: not connected");
        return false;
    }
}

bool BluetoothManager::broadcastMessage(const String &message)
{
    return sendMessage(message);
}

int BluetoothManager::getConnectedDeviceCount()
{
    if (serverRunning && pServer)
    {
        return pServer->getConnectedCount();
    }
    else if (clientConnected)
    {
        return 1;
    }
    return 0;
}