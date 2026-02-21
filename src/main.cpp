#include "esp_log.h"
#include <Arduino.h>
#include "esp_heap_caps.h"
#include "esp_spi_flash.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "connectivity/wifi/wifi.h"
#include "network/request.h"
#include "config/config.h"
#include "esp_wifi.h"

#include "daemon/daemon.h"
#include "network/gid.h"
#include "app.h"
#include "onlinelock/onlinelock.h"
#include "daemon/health/health.h"
#include "serial/log.h"
#include "unittest/math.h"
#include "unittest/memory.h"
#include "unittest/psram.h"
#include "unittest/storage.h"
#include "utility/init.h"
#include "unittest/initut.h"

#include "display/display.h"

#include "components/cyberpi/src/cyberpi.h"

#define APP_OPERATION_ID 0x01 // Operation ID for main app loop

static const char *TAG = "MAIN";
static const char *NET_TAG = "NET_DAEMON";

namespace
{
    using UnitTestFn = bool (*)();
    constexpr std::uint32_t kUnitTestTaskStackSize = 12288u;

    struct UnitTestTaskContext
    {
        volatile bool done;
        volatile bool allPassed;
    };

    bool runTimedUnitTest(const char *testName, UnitTestFn testFn, bool restartOnFail)
    {
        const std::int64_t startUs = esp_timer_get_time();
        const bool success = testFn();
        const std::int64_t elapsedMs = (esp_timer_get_time() - startUs) / 1000;

        if (success)
        {
            Log::sys_info(TAG, String(testName) + " successful (" + String(elapsedMs) + " ms)");
            return true;
        }

        Log::sys_error(TAG, String(testName) + " failed (" + String(elapsedMs) + " ms)");
        if (restartOnFail)
        {
            delay(2000);
            esp_restart();
        }

        return false;
    }

    void runUnitTestsTask(void *param)
    {
        auto *context = static_cast<UnitTestTaskContext *>(param);

        bool ok = true;
        ok = runTimedUnitTest("RAM Unit test", &UnitTest::runMemoryTest, true) && ok;
        ok = runTimedUnitTest("Storage Unit test", &UnitTest::runStorageTest, false) && ok;
        ok = runTimedUnitTest("PSRAM Unit test", &UnitTest::runPsramTest, false) && ok;
        ok = runTimedUnitTest("Math Unit test", &UnitTest::runMathTest, false) && ok;
        ok = runTimedUnitTest("Init Unit test", &UnitTest::initUnitTests, true) && ok;

        context->allPassed = ok;
        context->done = true;
        vTaskDelete(nullptr);
    }

    void logMemoryProfile()
    {
        const std::size_t internalTotal = heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        const std::size_t internalFree = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        const std::size_t psramTotal = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
        const std::size_t psramFree = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
        const std::size_t flashChip = spi_flash_get_chip_size();

        Log::sys_info(TAG, "Memory profile: internal total=" + String(internalTotal / 1024u) +
                               " KiB, internal free=" + String(internalFree / 1024u) + " KiB");
        Log::sys_info(TAG, "Memory profile: PSRAM total=" + String(psramTotal / 1024u) +
                               " KiB, PSRAM free=" + String(psramFree / 1024u) + " KiB");
        Log::sys_info(TAG, "Memory profile: SPI flash chip=" + String(flashChip / (1024u * 1024u)) + " MiB");
    }
} // namespace

// Global WiFi instance shared between main and daemon
WiFiConnect *g_wifi = nullptr;
String g_ssid;
String g_password;

// Global device configuration variables
int DEVICE_ID = 0;
String MAC_ADDR;

CyberPi cyber;
uint8_t samples[128];
int idx = 0;

// ESP-IDF native serial setup (before Arduino initialization)
static void setup_serial(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    esp_log_level_set(NET_TAG, ESP_LOG_INFO);
}

void init_app(void)
{
    // ESP-IDF native logging (no Arduino dependency)
    setup_serial();
    Log::sys_info(TAG, "=================================");
    Log::sys_info(TAG, "HkVLib Firmware Starting");
    Log::sys_info(TAG, "Build: " + String(BUILD) + ", Date: " + String(DATE) + ", Version: " + String(VERSION));
    Log::sys_info(TAG, "Free heap: " + String(esp_get_free_heap_size()) + " bytes");
    Log::sys_info(TAG, "=================================");

    // Initialize Arduino framework
    initArduino();
    Log::sys_info(TAG, "Arduino framework initialized");

    logMemoryProfile();

    // Init Display Component
    cyber.begin();
    Display::initialize();
    int font_size = 16;
    const wchar_t *titleText = L"HkVLib Firmware";
    Bitmap *bitmap = cyber.create_text(const_cast<wchar_t *>(titleText), 0xffff, font_size);
    cyber.set_bitmap(4, 4, bitmap);
    cyber.render_lcd();

    // Init Unit Test required for init phase (critical tests that must pass for safe operation, otherwise restart)
    // Unit tests for: RTOS, WiFi, RAM, Security

    // Init Configuration

    // Initialize Serial for reading commands
    Serial.begin(115200);
    Log::sys_info(TAG, "Serial initialized at 115200 baud");
    // Start serial input daemon for immediate command processing.
    Daemon::startSerialInputDaemon();

    delay(1000); // Brief delay to ensure Serial is ready
    Display::draw_boot(cyber);
    delay(2000);

    Display::draw_log(cyber, "Initializing WiFi...");

    // Initialize WiFi using Arduino library
    Log::sys_info(TAG, "Initializing WiFi...");
    g_wifi = new WiFiConnect();
    g_ssid = WLAN_SSID;
    g_password = WLAN_PASSWORD;

    if (g_wifi->connect(g_ssid, g_password))
    {
        Log::sys_info(TAG, "WiFi connected successfully");
        Log::sys_info(TAG, "IP Address: " + String(g_wifi->getLocalIP().c_str()));
        Display::draw_log(cyber, "WiFi initialization complete");
        delay(100);
        Display::draw_log(cyber, "IP Addr: " + String(g_wifi->getLocalIP().c_str()));
    }
    else
    {
        Log::sys_error(TAG, "WiFi connection failed! Attempting backup credentials...");
        Display::draw_log(cyber, "WiFi connection failed! Attempting backup credentials...");
        g_ssid = BACKUP_WLAN_SSID;
        g_password = BACKUP_WLAN_PASSWORD;
        if (g_wifi->connect(g_ssid, g_password))
        {
            Log::sys_info(TAG, "Backup WiFi connected successfully");
            Log::sys_info(TAG, "IP Address: " + String(g_wifi->getLocalIP().c_str()));
            Display::draw_log(cyber, "WiFi initialization complete");
            delay(100);
            Display::draw_log(cyber, "IP Addr: " + String(g_wifi->getLocalIP().c_str()));
        }
        else
        {
            Log::sys_error(TAG, "WiFi connection failed! Daemon will retry... with old credentials");
            Display::draw_log(cyber, "WiFi connection failed! Daemon will retry... with old credentials");
            g_ssid = WLAN_SSID;
            g_password = WLAN_PASSWORD;
        }
    }

    g_wifi->setDeviceName(DEVICE_NAME);

    Log::sys_info(TAG, "Setup complete. Starting background services...");

    // Start network daemon after WiFi init
    Log::sys_info(TAG, "Starting daemons...");
    Display::draw_log(cyber, "Starting daemons...");

    Daemon::startNetworkDaemon();
    Daemon::startHeartbeatDaemon();

    // Only load if feature is enabled.
    if (ONLINE_LOCK)
        Daemon::startOnlineLockDaemon();

    // Starting DHCP ID Client Configuration
    GID::gID();
    Daemon::startgIDDaemon();
    Daemon::startBluetoothDaemon();

    Display::draw_log(cyber, "Daemons started. Running unit tests...");
    // Unit Test (run in dedicated task to avoid main-task stack overflow)
    /**
     * @brief Unit test that are not nessecary at init time. Unit Test required to run are far higher.
     * p2p protocol, security, OTA, server communication, etc., mqtt, etc.
     */
    UnitTestTaskContext testContext = {false, false};
    BaseType_t created = xTaskCreatePinnedToCore(
        runUnitTestsTask,
        "unit_test_task",
        kUnitTestTaskStackSize,
        &testContext,
        tskIDLE_PRIORITY + 1,
        nullptr,
        xPortGetCoreID());

    if (created != pdPASS)
    {
        Log::sys_error(TAG, "Failed to create unit test task");
        Display::draw_log(cyber, "Failed to create unit test task - restarting...");
        delay(2000);
        esp_restart();
    }

    while (!testContext.done && optionalTest) // add timeout
    {
        delay(10);
    }
    if (!optionalTest)
        Log::sys_warning(TAG, "Skip optional test. Don't wait");

    if (!testContext.allPassed)
    {
        Log::sys_warning(TAG, "One or more non-critical unit tests failed");
        Display::draw_log(cyber, "One or more non-critical unit tests failed - check logs");
    }

    // Start Health Daemons
    HealthDaemons::startHealthDaemons();

    // TODO: Init Extensive Platform Cyper PI Lib

    // Init done
    Display::draw_log(cyber, "Initialization complete. Starting main loop...");
    delay(1000);
    Display::draw_team(cyber);
    delay(3000);
    Init::initialized();
}

extern "C" void app_main(void)
{
    init_app();
    App::init();
    OnlineLock::init();

    Log::sys_info(TAG, "Main loop starting on Core " + String(xPortGetCoreID()));
    while (true)
    {
        // Check if lock status changed and save state if interrupted
        if (OnlineLock::statusChanged())
        {
            if (OnlineLock::isLocked())
            {
                // Lock just engaged - save current state
                OnlineLock::saveProcessState(APP_OPERATION_ID, false);
                Log::sys_info(TAG, "App interrupted by online lock - state saved");
                return; // Exit immediately
            }
            else
            {
                // Lock just disengaged - resume from saved state
                if (OnlineLock::hasSavedState())
                {
                    ProcessState state = OnlineLock::getSavedState();
                    Log::sys_info(TAG, "App resuming from saved state (Op ID: " + String(state.operation_id) + ")");
                    OnlineLock::clearSavedState();
                }
            }
        }

        // If lock is active, skip processing
        if (OnlineLock::isLocked())
        {
            Log::sys_warning(TAG, "Waiting for online lock to be released...");
            delay(1000); // Brief pause to avoid busy loop
            return;
        }

        if (DEVICE_ID == -1)
        {
            Log::sys_error(TAG, "Device ID not yet assigned!");
            GID::gID(); // Attempt to request ID immediately instead of waiting for next heartbeat
            delay(5000);
            continue;
        }

        App::update();
    }
}