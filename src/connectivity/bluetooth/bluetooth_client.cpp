#include "bluetooth.h"
#include "../../serial/log.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

static const char *TAG = "BLUETOOTH";

bool BluetoothManager::scanForDevices(uint32_t duration)
{
    if (!initialized)
    {
        Log::sys_error(TAG, "Bluetooth not initialized");
        return false;
    }

    Log::sys_info(TAG, "Scanning for BLE devices...");
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);

    BLEScanResults foundDevices = pBLEScan->start(duration, false);
    int count = foundDevices.getCount();

    Log::sys_info(TAG, "Found " + String(count) + " devices");

    for (int i = 0; i < count; i++)
    {
        BLEAdvertisedDevice device = foundDevices.getDevice(i);
        String deviceInfo = "  [" + String(i) + "] " + device.getAddress().toString().c_str() + " (" + device.getName().c_str() + ") RSSI: " + String(device.getRSSI());
        Log::sys_info(TAG, deviceInfo);
    }

    pBLEScan->clearResults();
    return count > 0;
}

bool BluetoothManager::connectToDevice(const String &address)
{
    if (!initialized)
    {
        Log::sys_error(TAG, "Bluetooth not initialized");
        return false;
    }

    BLEAddress bleAddress(address.c_str());
    pClient = BLEDevice::createClient();

    if (!pClient)
    {
        Log::sys_error(TAG, "Failed to create BLE client");
        return false;
    }

    Log::sys_info(TAG, "Connecting to " + address);

    if (!pClient->connect(bleAddress))
    {
        Log::sys_error(TAG, "Failed to connect");
        return false;
    }

    Log::sys_info(TAG, "Connected to device");

    BLERemoteService *pRemoteService = pClient->getService(SERVICE_UUID);
    if (pRemoteService == nullptr)
    {
        Log::sys_error(TAG, "Failed to find service UUID");
        pClient->disconnect();
        return false;
    }

    pRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);
    if (pRemoteCharacteristic == nullptr)
    {
        Log::sys_error(TAG, "Failed to find characteristic UUID");
        pClient->disconnect();
        return false;
    }

    clientConnected = true;

    if (connectionCallback)
    {
        connectionCallback(address, true);
    }

    return true;
}

bool BluetoothManager::disconnectFromDevice()
{
    if (clientConnected && pClient)
    {
        pClient->disconnect();
        clientConnected = false;
        Log::sys_info(TAG, "Disconnected from device");

        if (connectionCallback)
        {
            connectionCallback("", false);
        }

        return true;
    }
    return false;
}

bool BluetoothManager::isConnected()
{
    return clientConnected && pClient && pClient->isConnected();
}

int BluetoothManager::getRSSI()
{
    if (clientConnected && pClient)
    {
        return pClient->getRssi();
    }
    return 0;
}