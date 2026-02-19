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
    Log::sys_info(TAG, "Free heap: " + String(esp_get_free_heap_size()) + " bytes");
    Log::sys_info(TAG, "=================================");

    // Initialize Arduino framework
    initArduino();
    Log::sys_info(TAG, "Arduino framework initialized");

    logMemoryProfile();

    // Init Display Component

    // Init Configuration

    // Initialize Serial for reading commands
    Serial.begin(115200);
    Log::sys_info(TAG, "Serial initialized at 115200 baud");

    // Initialize WiFi using Arduino library
    Log::sys_info(TAG, "Initializing WiFi...");
    g_wifi = new WiFiConnect();
    g_ssid = WLAN_SSID;
    g_password = WLAN_PASSWORD;

    if (g_wifi->connect(g_ssid, g_password))
    {
        Log::sys_info(TAG, "WiFi connected successfully");
        Log::sys_info(TAG, "IP Address: " + String(g_wifi->getLocalIP().c_str()));
    }
    else
    {
        Log::sys_error(TAG, "WiFi connection failed! Attempting backup credentials...");
        g_ssid = BACKUP_WLAN_SSID;
        g_password = BACKUP_WLAN_PASSWORD;
        if (g_wifi->connect(g_ssid, g_password))
        {
            Log::sys_info(TAG, "Backup WiFi connected successfully");
            Log::sys_info(TAG, "IP Address: " + String(g_wifi->getLocalIP().c_str()));
        }
        else
        {
            Log::sys_error(TAG, "WiFi connection failed! Daemon will retry... with old credentials");
            g_ssid = WLAN_SSID;
            g_password = WLAN_PASSWORD;
        }
    }

    Log::sys_info(TAG, "Setup complete. Starting background services...");

    // Start network daemon after WiFi init
    Log::sys_info(TAG, "Starting daemons...");

    Daemon::startNetworkDaemon();
    Daemon::startHeartbeatDaemon();

    // Only load if feature is enabled.
    if (ONLINE_LOCK)
        Daemon::startOnlineLockDaemon();

    // Start serial input daemon for immediate command processing.
    Daemon::startSerialInputDaemon();

    // Starting DHCP ID Client Configuration
    GID::gID();
    Daemon::startgIDDaemon();

    // Unit Test (run in dedicated task to avoid main-task stack overflow)
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
        delay(2000);
        esp_restart();
    }

    while (!testContext.done)
    {
        delay(10);
    }

    if (!testContext.allPassed)
    {
        Log::sys_warning(TAG, "One or more non-critical unit tests failed");
    }

    delay(2000); // Brief pause before starting main loop

    // Start Health Daemons
    HealthDaemons::startHealthDaemons();

    // Init Extensive Platform

    // Init done
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
            delay(5000);
            continue;
        }

        App::update();
    }
}