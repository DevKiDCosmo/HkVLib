# Unit Tests - Onboard Testing (Bootloader & Runtime)

## Unit Test Stages

There are two priority levels for unit tests:

- **PRIO-0**: System-critical modules that must pass for the system to be considered operational. Failure of any PRIO-0 test indicates a critical system fault.
- **PRIO-1**: Non-critical verification tests that run after all necessary daemons are initialized. These tests verify functionality and performance but can fail without preventing system operation.

## Currently Implemented Tests

### PRIO-0 (Critical)
- **RAM**: Memory allocation and integrity verification
- **PSRAM**: External PSRAM availability and functionality
- [ ] **INIT**: Check if Init is Tempered (Needs to be enhance)

### PRIO-1 (Non-Critical)
- **Math**: Benchmark and floating-point precision tests
- **Storage**: Flash storage read/write operations

## Planned Tests

### PRIO-0 (Critical System Tests)
- [ ] **RTOS Verification**: FreeRTOS task scheduling, semaphores, queues, and timers
- [ ] **Daemon Verification**: Ensure all critical daemons start and respond correctly
  - [ ] gIDDaemon (Global ID management)
  - [ ] HealthDaemons (System health monitoring)
  - [ ] HeartbeatDaemon (Server Reachability monitoring)
  - [ ] NetworkDaemon (Network connectivity)
  - [ ] OnlineLockDaemon (Resource locking)
  - [ ] SerialInputDaemon (Serial communication)
- [ ] **Calibration**: Sensor calibration data integrity and validity
- [ ] **CyberPI Bridge**: Communication bridge to chassis hardware
- [ ] **Network**: Basic network stack initialization
- [ ] **I2C/SPI Bus**: Hardware bus initialization and communication
- [ ] **GPIO**: Pin configuration and basic I/O operations
- [ ] **Watchdog Timer**: Hardware watchdog functionality
- [ ] **Clock System**: System clock and timer accuracy
- [ ] **Interrupt Handler**: Critical interrupt registration and handling
- [ ] **Boot Partition**: Verify boot partition integrity and rollback capability
- [ ] **Panick System**: This checks if interruptions are possible.

### PRIO-1 (Feature & Integration Tests)
- [ ] **WiFi Connectivity**: WiFi initialization, connection, and reconnection
- [ ] **Bluetooth**: BLE/Classic Bluetooth pairing and communication
- [ ] **MQTT Client**: Message publishing and subscription
- [ ] **P2P Protocol**: Peer-to-peer communication protocol
- [ ] **Network Requests**: HTTP/HTTPS request and response handling
- [ ] **Display**: LCD/OLED display initialization and basic rendering
- [ ] **Identity Management**: Device ID generation and storage
- [ ] **Online Lock**: Distributed locking mechanism
- [ ] **Serial Logging**: Log output formatting and buffering
- [ ] **Mission System**: Mission loading and execution
- [ ] **Configuration**: Config file parsing and validation (YAML/JSON)
- [ ] **Utility Functions**: Hex conversion, string manipulation
- [ ] **Broadcast**: Message broadcasting functionality
- [ ] **Health Monitoring**: System health metrics collection
- [ ] **Gyro/Accelerometer**: IMU sensor reading and calibration
- [ ] **Microphone**: Audio input sampling and processing
- [ ] **Sound Output**: Audio playback functionality
- [ ] **LCD Color Display**: Advanced graphics and rendering

### Performance & Stress Tests
- [ ] **Memory Leak Detection**: Long-running memory allocation tests
- [ ] **Task Switching Overhead**: RTOS context switch performance
- [ ] **Network Throughput**: WiFi/Bluetooth data transfer rates
- [ ] **Storage I/O Performance**: Flash read/write speed benchmarks
- [ ] **Power Consumption**: Current draw measurements per component
- [ ] **Concurrent Task Load**: Multiple simultaneous operations
- [ ] **Interrupt Latency**: Response time for critical interrupts
- [ ] **Queue Saturation**: Message queue overflow handling

## Test Structure

- Tests are organized by component/module in subdirectories
- Each test file corresponds to a specific module in the main codebase
- Test files follow the naming convention: `<module_name>.cpp` or `<module_name>.h`
- Tests output results via `Log::` functions for serial monitoring

## Test Guidelines

### Development Rules
- Write clear, descriptive test names that explain what is being tested
- **Do not** initialize or depend on daemons in PRIO-0 tests
- Use `Log::` functions for all test output (no direct Serial access)
- Structure tests in two stages:
  - **Init Verification Stage**: Run during system initialization
  - **UnitTest Verification Stage**: Run after system is fully initialized (not yet implemented)

### Best Practices
- Keep tests isolated and independent
- Test both success and failure cases
- Include boundary value testing
- Document expected behavior and known limitations
- Use meaningful assertions with descriptive error messages
- Clean up resources after tests (free memory, close handles, etc.)
- Consider edge cases and error conditions
- Test timeout and recovery mechanisms

### Execution Order
1. **PRIO-0 tests** run during bootloader/early init stage
2. System initialization continues only if all PRIO-0 tests pass
3. Daemons and services start
4. **PRIO-1 tests** run after system is fully operational

## Future Enhancements

- [ ] Implement UnitTest Verification Stage framework
- [ ] Add automated test execution on build (PlatformIO test framework)
- [ ] Create test report generation (JSON/XML output)
- [ ] Implement mock objects for external hardware dependencies
- [ ] Add continuous integration (CI) pipeline integration
- [ ] Create test coverage analysis tools
- [ ] Develop hardware-in-the-loop (HIL) testing setup
- [ ] Add regression test suite
- [ ] Implement fuzz testing for protocol handlers
- [ ] Create test documentation with examples for each module
- [ ] Add test result persistence and comparison over builds
- [ ] Implement remote test execution and monitoring
- [ ] Add automated failure diagnostics and suggestions

## Daemon Unit Test
Watchdog, Mem leackage, exception + failure.

## Passive Unit Tests (Build-Time Only)

These tests run during compilation/build and verify code correctness without requiring hardware:
- Static analysis checks
- Compile-time assertions
- Configuration validation
- Memory layout verification
- Dead code detection
- Dependency graph validation