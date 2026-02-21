#include "bluetooth.h"
#include "../../serial/log.h"
#include <Arduino.h>
#include <BLEDevice.h>

#if __has_include(<esp_ble_api.h>)
#include <esp_ble_api.h>
#define HKV_HAS_ESP_TX_POWER_API 1
#elif __has_include(<esp_gap_ble_api.h>)
#include <esp_gap_ble_api.h>
#define HKV_HAS_ESP_TX_POWER_API 1
#else
#define HKV_HAS_ESP_TX_POWER_API 0
#endif

static const char *TAG = "BLUETOOTH";

// Constructor
BluetoothManager::BluetoothManager()
    : deviceName(""),
      macAddress(""),
      initialized(false),
      serverRunning(false),
      clientConnected(false),
      pServer(nullptr),
      pCharacteristic(nullptr),
      pClient(nullptr),
      pRemoteCharacteristic(nullptr),
      messageCallback(nullptr),
      connectionCallback(nullptr)
{
}

// Destructor
BluetoothManager::~BluetoothManager()
{
    end();
}

// Initialize Bluetooth
bool BluetoothManager::begin(const String &name)
{
    if (initialized)
    {
        Log::sys_warning(TAG, "Bluetooth already initialized");
        return true;
    }

    deviceName = name;

    BLEDevice::init(deviceName.c_str());

    // Get MAC address
    macAddress = String(BLEDevice::getAddress().toString().c_str());
    Log::sys_info(TAG, "Bluetooth initialized: " + deviceName + " [" + macAddress + "]");

    initialized = true;
    return true;
}

// Shutdown Bluetooth
void BluetoothManager::end()
{
    stopServer();
    disconnectFromDevice();

    if (initialized)
    {
        BLEDevice::deinit(true);
        initialized = false;
        Log::sys_info(TAG, "Bluetooth shutdown");
    }
}

// Get device name
String BluetoothManager::getDeviceName()
{
    return deviceName;
}

// Get MAC address
String BluetoothManager::getMacAddress()
{
    return macAddress;
}

// Set message callback
void BluetoothManager::onMessageReceived(MessageCallback callback)
{
    messageCallback = callback;
}

// Set connection callback
void BluetoothManager::onDeviceConnection(ConnectionCallback callback)
{
    connectionCallback = callback;
}

// Get connected devices
std::vector<String> BluetoothManager::getConnectedDevices()
{
    return connectedDevices;
}

// Get TX power
uint8_t BluetoothManager::getTxPower()
{
#if HKV_HAS_ESP_TX_POWER_API
    return esp_ble_tx_power_get(ESP_BLE_PWR_TYPE_DEFAULT);
#else
    return 0;
#endif
}

// Check if initialized
bool BluetoothManager::isInitialized()
{
    return initialized;
}
