# P2P Protocol Documentation

## Overview

The P2P (Peer-to-Peer) Protocol enables direct communication between multiple devices using Bluetooth Low Energy (BLE). It's designed for MBOT and other IoT devices to communicate, discover each other, and coordinate missions.

## Message Format

All P2P messages follow this format:

```
DEVICETYPE:MAC:TYPE:SEQUENCE:PAYLOAD
```

### Fields:
- **DEVICETYPE**: Type of device (e.g., "MBOT", "ROBOT", "SENSOR")
- **MAC**: Sender's MAC address
- **TYPE**: Message type (PING, PONG, DISCOVER, etc.)
- **SEQUENCE**: Sequential message number
- **PAYLOAD**: Message-specific data

### Example Messages:
```
MBOT:24:0A:C4:12:34:56:PING:1:
MBOT:24:0A:C4:12:34:56:PONG:2:response_data
MBOT:24:0A:C4:12:34:56:MISSION:3:mission_id:12345|type:navigate
MBOT:24:0A:C4:12:34:56:STATUS:4:online|heap:123456|uptime:3600
```

## Message Types

| Type | Value | Description | Use Case |
|------|-------|-------------|----------|
| PING | 0x01 | Request connection test | Test device connectivity |
| PONG | 0x02 | Response to PING | Confirm connectivity and measure latency |
| DISCOVER | 0x03 | Device discovery request | Find other devices in range |
| DISCOVER_RESPONSE | 0x04 | Response to discovery | Announce presence and capabilities |
| DATA | 0x05 | General data transfer | Send arbitrary data |
| ACK | 0x06 | Acknowledgment | Confirm message receipt |
| MISSION | 0x07 | Mission-related data | Coordinate missions between devices |
| STATUS | 0x08 | Status update | Broadcast device health/status |
| ERROR | 0x09 | Error notification | Report errors |

## Quick Start

### 1. Include Headers

```cpp
#include "connectivity/bluetooth/bluetooth.h"
#include "p2p/protocol.h"
```

### 2. Initialize

```cpp
BluetoothManager bluetooth;
P2PProtocol p2p(&bluetooth);

void setup() {
    // Initialize Bluetooth
    bluetooth.begin("MBOT_001");
    bluetooth.startServer();
    
    // Initialize P2P
    p2p.begin("MBOT");
    
    // Register callbacks
    p2p.onMessageReceived(onMessage);
    p2p.onDeviceDiscovered(onDiscovery);
    p2p.onPingResponse(onPing);
}
```

### 3. Discover Devices

```cpp
// Start discovery for 10 seconds
p2p.startDiscovery(10);

// Get discovered devices
auto devices = p2p.getDiscoveredDevices();
for (const auto &device : devices) {
    Serial.printf("Found: %s [%s]\n", 
                  device.name.c_str(), 
                  device.macAddress.c_str());
}
```

### 4. Send Messages

```cpp
// Send ping to specific device
p2p.sendPing("24:0a:c4:12:34:56");

// Send custom data
p2p.sendMessage("24:0a:c4:12:34:56", 
                P2PMessageType::DATA, 
                "Hello World");

// Broadcast to all devices
p2p.broadcastMessage(P2PMessageType::STATUS, 
                     "online|ready");
```

### 5. Handle Callbacks

```cpp
void onMessage(const P2PMessage &msg, const P2PDevice &device) {
    Serial.printf("Message from %s: %s\n", 
                  device.macAddress.c_str(), 
                  msg.payload.c_str());
}

void onDiscovery(const P2PDevice &device) {
    Serial.printf("Discovered: %s\n", device.name.c_str());
}

void onPing(const String &deviceMAC, uint32_t latency) {
    Serial.printf("Ping to %s: %d ms\n", 
                  deviceMAC.c_str(), 
                  latency);
}
```

## Use Cases

### 1. Device Testing & Monitoring

```cpp
// Test connection to all devices
void testAllDevices() {
    auto devices = p2p.getDiscoveredDevices();
    for (const auto &device : devices) {
        p2p.testConnection(device.macAddress);
    }
}

// Monitor latency
p2p.onPingResponse([](const String &mac, uint32_t latency) {
    if (latency > 100) {
        Serial.println("High latency detected!");
    }
});
```

### 2. Mission Coordination

```cpp
// Send mission to specific device
void assignMission(const String &targetMAC, const String &missionId) {
    String payload = "mission_id:" + missionId + 
                    "|type:navigate|target:waypoint_A";
    p2p.sendMissionData(targetMAC, payload);
}

// Broadcast mission to all devices
void broadcastMission(const String &missionData) {
    p2p.broadcastMissionData(missionData);
}

// Handle incoming mission
void onMessage(const P2PMessage &msg, const P2PDevice &device) {
    if (msg.type == P2PMessageType::MISSION) {
        // Parse and execute mission
        processMission(msg.payload);
    }
}
```

### 3. Status Broadcasting

```cpp
// Periodic status update
void sendStatusUpdate() {
    String status = "online|" + 
                   "heap:" + String(ESP.getFreeHeap()) + "|" +
                   "uptime:" + String(millis() / 1000) + "|" +
                   "battery:85";
    p2p.sendStatus(status);
}
```

### 4. Swarm Behavior

```cpp
// Coordinate multiple devices
void coordinateSwarm() {
    auto devices = p2p.getDiscoveredDevices();
    
    // Assign roles to each device
    for (int i = 0; i < devices.size(); i++) {
        String role = "role:" + String(i) + "|formation:line";
        p2p.sendMessage(devices[i].macAddress, 
                       P2PMessageType::DATA, 
                       role);
    }
}
```

## API Reference

### BluetoothManager

#### Methods

- `bool begin(const String &deviceName)` - Initialize Bluetooth
- `bool startServer()` - Start BLE server mode
- `bool connectToDevice(const String &address)` - Connect to specific device
- `bool sendMessage(const String &message)` - Send message
- `String getMacAddress()` - Get local MAC address
- `int getRSSI()` - Get signal strength

### P2PProtocol

#### Methods

- `bool begin(const String &deviceType)` - Initialize P2P protocol
- `void startDiscovery(uint32_t durationSec)` - Start device discovery
- `bool sendPing(const String &targetMAC)` - Ping specific device
- `bool sendMessage(const String &targetMAC, P2PMessageType type, const String &payload)` - Send message
- `bool broadcastMessage(P2PMessageType type, const String &payload)` - Broadcast message
- `std::vector<P2PDevice> getDiscoveredDevices()` - Get device list
- `void printDeviceList()` - Print discovered devices
- `void printStatistics()` - Print P2P statistics

#### Callbacks

- `void onMessageReceived(MessageHandler handler)` - Register message handler
- `void onDeviceDiscovered(DeviceDiscoveryHandler handler)` - Register discovery handler
- `void onPingResponse(PingResponseHandler handler)` - Register ping handler

## Best Practices

### 1. Resource Management

```cpp
// Clean up inactive devices periodically
void loop() {
    p2p.clearInactiveDevices(60000); // 60 second timeout
    delay(1000);
}
```

### 2. Error Handling

```cpp
if (!bluetooth.begin("MBOT_001")) {
    Log::sys_error(TAG, "Bluetooth init failed");
    return;
}

if (!p2p.sendMessage(mac, type, payload)) {
    Log::sys_error(TAG, "Failed to send message");
    // Retry or handle error
}
```

### 3. Periodic Tasks

```cpp
void p2pTask(void *param) {
    while (true) {
        // Periodic ping every 5 seconds
        static uint32_t lastPing = 0;
        if (millis() - lastPing >= 5000) {
            lastPing = millis();
            pingAllDevices();
        }
        
        // Discovery every 30 seconds
        static uint32_t lastDiscovery = 0;
        if (millis() - lastDiscovery >= 30000) {
            lastDiscovery = millis();
            p2p.startDiscovery(5);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

### 4. Performance Monitoring

```cpp
void checkPerformance() {
    uint32_t avgLatency = p2p.getAveragePingLatency();
    if (avgLatency > 200) {
        Log::sys_warning(TAG, "High average latency: %d ms", avgLatency);
    }
    
    int deviceCount = p2p.getDeviceCount();
    Log::sys_info(TAG, "Connected devices: %d", deviceCount);
}
```

## Troubleshooting

### Device Not Discovered

1. Check Bluetooth is initialized: `bluetooth.isInitialized()`
2. Ensure server is running: `bluetooth.isServerRunning()`
3. Increase scan duration: `p2p.startDiscovery(30)`
4. Check signal strength (RSSI should be > -80 dBm)

### Message Not Received

1. Verify connection: `bluetooth.isConnected()`
2. Check message format is correct
3. Ensure callbacks are registered
4. Monitor for errors in logs

### High Latency

1. Reduce distance between devices
2. Minimize interference (WiFi, other BLE devices)
3. Check TX power: `bluetooth.getTxPower()`
4. Monitor RSSI values

## Future Extensions

The P2P protocol is designed to be extensible for:

- **Mission Module**: Advanced mission coordination and execution
- **Formation Control**: Swarm formation and movement
- **Data Synchronization**: State synchronization between devices
- **Mesh Networking**: Multi-hop message routing
- **Security**: Encrypted communications

## License

Part of HkVLib project
