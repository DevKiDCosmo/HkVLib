# HkVLib - Hackathon V Library

ESP32-WROVER-B Firmware Library for mBot2 Robotics Competition

## Overview

HkVLib is a hybrid C++/Python firmware library designed for managing 30+ mBot2 robots in a hackathon environment. It provides real-time control, MQTT-based communication, auto-provisioning, and automatic verification systems.

## Stoarage & Mem Specs
Onboard memory	ROM	448 KB
SRAM	520 KB
Extended memory	SPI Flash	8 MB
PSRAM	8 MB

## System Architecture

```
mBot2 (C++ Core Firmware)
    ↓
Python Wrapper (CyberPi Layer)
    ↓
MQTT Protocol
    ↓
Mac Mini Server (Broker + Game Logic + Verification)
```

### Hardware Specifications
- **Controller**: ESP32-WROVER-B (520KB RAM, 16MB Flash)
- **Platform**: mBot2 with CyberPi
- **Network**: WiFi 802.11b/g/n + P2P Mesh Fallback
- **Communication**: MQTT over TCP/IP

## MQTT Topic Structure

```
mbot/<id>/status        # Device status reports
mbot/<id>/telemetry     # Sensor data streaming
mbot/<id>/cmd           # Command reception
mbot/<id>/test          # Self-test results
mbot/<id>/fault         # Fault detection alerts
mbot/broadcast          # Global announcements
mbot/register           # Auto ID registration
mbot/presence           # LAN discovery (heartbeat)
mbot/npc/#              # NPC bot topics
mbot/player/#           # Player bot topics
```

## Core Library Modules

### 1. WiFi Management

**C++ API:**
```cpp
wifiInit()                    // Initialize WiFi module
wifiAutoConnect()             // Auto-connect to configured network
wifiReconnect()               // Force reconnection
wifiSignalStrength()          // Get RSSI value
wifiStatus()                  // Connection status
```

**Python API:**
```python
auto_connect_wifi(ssid, password)
get_ip()
get_mac()
reconnect_wifi()
```

**Features:**
- Automatic connection to pre-configured networks
- Signal strength monitoring
- Fallback to P2P mesh if infrastructure WiFi fails
- Connection state persistence

### 2. Auto ID Provisioning System

**Goal:** Bots receive automatic ID assignment from server

**ID Types:**

| ID Type | Description | Configurable | Set By | Example |
|---------|-------------|--------------|---------|---------|
| **TEAMID** | Team identifier | ❌ NO | Registration (before handout) | `TEAM-42` |
| **DEVICEID** | Device identifier (like IP) | ❌ NO | DHCP ID System | `MBOT-07` |
| **Name** | Display name | ✅ YES | User | `CyberBot-Alpha` |

**TEAMID:**
- Set during team registration before the hackathon
- Hardcoded into firmware at handout
- Cannot be changed by contestants
- Used for team-based verification and scoring

**DEVICEID:**
- Assigned automatically via DHCP-like ID system
- Like an IP address - dynamically allocated on first connection
- Based on MAC address + Role (npc/player)
- Stored in EEPROM for persistence across reboots

**Name:**
- Can be changed by contestants
- Used for display purposes in dashboard
- Not used for identification/verification

**Process:**
1. Bot connects to WiFi
2. Sends MAC address + Role (npc/player)
3. Server assigns unique DEVICEID
4. Bot stores DEVICEID in flash memory (EEPROM)
5. TEAMID is already pre-configured in firmware

**C++ API:**
```cpp
requestID()                   // Request ID from server
storeIDtoEEPROM()             // Persist ID to flash
loadIDfromEEPROM()            // Load ID on boot
resetID()                     // Clear stored ID
```

**Python API:**
```python
request_id_from_server()
publish_identity()
set_device_role(role)         # "npc" or "player"
```

**MQTT Registration:**
- **Topic:** `mbot/register`
- **Payload:**
```json
{
  "mac": "AA:BB:CC:DD:EE:FF",
  "role": "npc"
}
```

### 3. Device Naming & LAN Discovery

**Naming Schema:**
- NPC Bots: `MBOT2-NPC-07`
- Player Bots: `MBOT2-PLAYER-03`

**C++ API:**
```cpp
setDeviceName()               // Configure device name
getDeviceName()               // Retrieve device name
publishPresence()             // Broadcast presence on LAN
heartbeat()                   // Send periodic heartbeat
```

**MQTT Topic:** `mbot/presence`
- **Frequency:** Every 5 seconds
- **Purpose:** LAN discovery and health monitoring

### 4. Motor Control (Real-time C++)

**C++ API:**
```cpp
motorInit()                   // Initialize motor drivers
motorDrive(left, right)       // Drive with left/right speeds (-255 to 255)
motorStop()                   // Emergency stop
motorDistance(cm)             // Drive specific distance
motorTurn(degree)             // Turn by degrees
motorPIDEnable()              // Enable PID control
motorPIDConfig(kp, ki, kd)    // Configure PID parameters
```

### 5. Sensor Layer

**C++ API:**
```cpp
imuInit()                     // Initialize IMU
imuRead()                     // Read accelerometer/gyroscope
encoderRead()                 // Read wheel encoders
ultrasonicRead()              // Read ultrasonic distance sensor
lineSensorRead()              // Read line following sensors
batteryLevel()                // Get battery voltage
```

**Python Wrapper:**
```python
get_position()                # Get (x, y) coordinates
get_angle()                   # Get heading angle
get_distance()                # Get ultrasonic distance
get_line_state()              # Get line sensor array state
```

### 6. Unit Test System (Critical for 30+ Bots)

**Automated Boot Tests:**

Test sequence runs automatically on boot:

1. **testSensors()**
   - IMU Check (motion detection)
   - Encoder Check (wheel rotation)
   - Ultrasonic Check (distance measurement)
   - Line Sensor Check (sensor array)

2. **testMotors()**
   - 1 second left rotation
   - 1 second right rotation
   - Encoder comparison validation

3. **testMemory()**
   - Heap free check
   - Stack integrity check
   - Flash read/write test

4. **testMQTT()**
   - Publish + Echo verification
   - Broker connectivity test

**Test Results:**
- **Topic:** `mbot/<id>/test`
- **Payload:**
```json
{
  "imu": true,
  "encoder": true,
  "motor": true,
  "memory": true,
  "mqtt": true,
  "timestamp": 1234567890
}
```

**C++ API:**
```cpp
testSensors()
testMotors()
testMemory()
testMQTT()
runAllTests()                 // Execute complete test suite
```

### 7. Fault Detection System

**C++ API:**
```cpp
watchdogEnable()              // Enable hardware watchdog
memoryCheck()                 // Monitor heap/stack usage
motorStallDetect()            // Detect motor stalls
sensorTimeoutDetect()         // Detect sensor timeouts
emergencyStop()               // Emergency halt all systems
```

**Fault Reports:**
- **Topic:** `mbot/<id>/fault`
- **Triggers:** Watchdog timeout, memory exhaustion, sensor failure, motor stall

### 8. Swarm Utilities

**C++ API:**
```cpp
syncTimeFromServer()          // Synchronize system clock
setMissionState(state)        // Update mission state - block. Vulnerability issue and potential for exploitation
getMissionState()             // Retrieve current state
npcBehaviorMode(mode)         // Set NPC behavior
```

**Python API:**
```python
subscribe_command()
execute_command()
broadcast_state()
```

## Python MQTT Wrapper

```python
from umqtt.simple import MQTTClient
import json

class HackathonMQTT:
    def __init__(self, client_id, broker):
        self.client = MQTTClient(client_id, broker)

    def connect(self):
        self.client.connect()

    def publish(self, topic, payload):
        self.client.publish(topic, json.dumps(payload))

    def subscribe(self, topic, callback):
        self.client.set_callback(callback)
        self.client.subscribe(topic)

    def loop(self):
        self.client.check_msg()
```

## Boot Sequence State Machine

```
[BOOT]
  ↓
[INIT_HARDWARE]
  ↓
[WIFI_CONNECT] ──► (fail) ──► [P2P_MESH_MODE] ──► [Try to connect to server through other bot]
  ↓ (success)
[REQUEST_ID]
  ↓
[LOAD_CONFIG]
  ↓
[RUN_SELF_TESTS]
  ↓
[START_DAEMONS]
  │   ├── Network Daemon (ND)
  │   ├── Debug Daemon (DD)
  │   ├── Verification Daemon (VD)
  │   ├── Unit Test Task (UTT)
  │   └── System Daemon (SD)
  ↓
[APPLICATION_START]
  ↓
[MISSION_LOOP]
```

## Daemons and Background Tasks

### Network Daemon (ND)
- **Purpose:** Maintain WiFi connectivity
- **Action:** Auto-reconnect on disconnect
- **Fallback:** Switch to P2P mesh mode

### Debug Daemon (DD)
- **Purpose:** Remote debugging and logging
- **Output:** Serial + MQTT topic `mbot/<id>/debug`

### Verification Daemon (VD)
- **Purpose:** Automatic competition verification
- **Checks:** Mission completion, rule compliance

### Unit Test Task (UTT)
- **Purpose:** Continuous health monitoring
- **Schedule:** Runs at init

### System Daemon (SD)
- **Purpose:** Resource management and fault recovery
- **Monitors:** Memory, CPU, battery, temperature

## Server Requirements (Mac Mini)

**Required Components:**
1. **MQTT Broker** (Mosquitto)
2. **ID Registry** (Auto-provisioning service)
3. **Role Manager** (NPC/Player assignment)
4. **Mission Engine** (Game logic)
5. **NPC AI** (NPC behavior controller)
6. **Live Dashboard** (Real-time monitoring)
7. **Fault Monitor** (Error aggregation)

## Safety Mechanisms

**For 30+ Concurrent Bots:**
- **Rate Limiting:** Max 10 publishes/second per bot
- **Offline Timeout:** Remove disconnected bots after 30s
- **Duplicate ID Check:** Prevent ID conflicts
- **Token-based Registration:** Secure provisioning
- **TLS Support:** Optional encryption (recommended)

## Configuration

**WiFi Credentials:**
Edit `include/wifi_config.h`:
```cpp
#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#define WIFI_SSID "YourNetworkName"
#define WIFI_PASSWORD "YourNetworkPassword"

#endif
```

**MQTT Broker:**
Configure in main application:
```cpp
const string mqtt_broker = "192.168.1.100";
int mqtt_port = 1883;
```

## File Structure

```
HkVLib/
├── src/
│   ├── main.cpp              # Application entry point
│   ├── connectivity/
│   │   ├── wifi/             # WiFi management
│   │   └── mqtt/             # MQTT client
│   ├── hardware/
│   │   ├── motor.cpp         # Motor control
│   │   └── sensor.cpp        # Sensor interface
│   ├── system/
│   │   ├── daemon.cpp        # Background tasks
│   │   ├── test.cpp          # Unit tests
│   │   └── fault.cpp         # Fault detection
│   └── identity/             # ID management
├── include/
│   └── wifi_config.h         # WiFi credentials
├── lib/                      # External libraries
└── platformio.ini            # Build configuration
```

## Python Layer (CyberPi)

**Module Structure:**
```
python/
├── network_manager.py        # WiFi connection management
├── mqtt_client.py           # MQTT communication
├── mission_logic.py         # Mission-specific code
├── npc_ai.py               # NPC behavior patterns
└── diagnostics.py          # Health monitoring
```

## Build Instructions

**Requirements:**
- PlatformIO IDE or CLI
- ESP32 toolchain

**Build:**
```bash
pio run
```

**Upload:**
```bash
pio run --target upload
```

**Monitor:**
```bash
pio device monitor
```

## API Quick Reference

### C++ Core Library
```cpp
// Initialization
hackathonInit(); // Runs semi-async. If it finishs to defined point. Async is active.
// Deamons are running in the background.

// WiFi
// wifiAutoConnect();

// Identity
// requestID();
// getDeviceName();

// Motor

motorDrive(255, 255);  // Full speed forward
motorTurn(90);         // Turn 90 degrees

// Sensor
int dist = ultrasonicRead();
float heading = imuRead();

// MQTT
mqttPublish("mbot/1/status", jsonPayload);
```

### Python Mission API
```python
from hackathon import *

# Initialize
bot = HackathonBot()

# Drive
bot.motor_drive(100, 100)

# Read sensors
position = bot.get_position()
distance = bot.get_distance()

# MQTT
bot.publish_status({"mission": "running"})
```

## Troubleshooting

**Serial not working:**
- Ensure `.cpp` extension (not `.c`)
- Use `Serial.begin(115200)` matching `platformio.ini`
- Arduino `Serial` requires `initArduino()` call first

**WiFi connection fails:**
- Check credentials in `wifi_config.h`
- Verify WiFi signal strength
- Check for MAC address filtering

**MQTT connection fails:**
- Verify broker IP address
- Check firewall settings
- Ensure unique client ID

**Out of memory:**
- Reduce MQTT buffer sizes
- Optimize task stack sizes
- Enable PSRAM if available

## License

MIT License - Hackathon V Competition

## Support

For technical issues during competition:
1. Check self-test results on `mbot/<id>/test`
2. Review fault logs on `mbot/<id>/fault`
3. Contact technical staff via competition channel



## More function.
Health and Battery also
Serial Command Palette