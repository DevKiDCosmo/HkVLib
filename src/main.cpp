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
#include "unittest/components/certification/certification.h"
#include "unittest/components/RTOS/RTOS.h"
#include "unittest/components/fs/fs.h"
#include "unittest/corrupt/corrupt.h"
#include "utility/init.h"
#include "unittest/initut.h"

#include "display/display.h"
#include "nvs/nvs.h"
#include "components/cyberpi/src/cyberpi.h"

#define APP_OPERATION_ID 0x01 // Operation ID for main app loop

static const char *TAG = "MAIN";
static const char *NET_TAG = "NET_DAEMON";

namespace
{
    using UnitTestFn = bool (*)();
    constexpr std::uint32_t kUnitTestTaskStackSize = 12288u;
    constexpr std::uint32_t kForceRequiredUnitTestsNow = 0xFFFFFFFFu;

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
        ok = runTimedUnitTest("Filesystem Unit test", &UnitTest::runFsMainCheck, false) && ok;
        ok = runTimedUnitTest("PSRAM Unit test", &UnitTest::runPsramTest, false) && ok;
        ok = runTimedUnitTest("Math Unit test", &UnitTest::runMathTest, false) && ok;
        // ok = runTimedUnitTest("Certification Unit test", &UnitTest::runCertificationSuite, false) && ok;

        context->allPassed = ok;
        context->done = true;
        vTaskSuspend(nullptr);
    }

    bool runRequiredUnitTests(std::uint32_t circle, std::uint32_t lastRequiredRun)
    {
        if (lastRequiredRun == kForceRequiredUnitTestsNow)
        {
            Log::sys_info(TAG, "RUT force flag detected, running required unit tests now");
        }
        else if (lastRequiredRun <= circle)
        {
            NVSStore::setUInt(NVSKey::Key::LastRequiredUnitTest, lastRequiredRun + 1u);
            return true;
        }

        bool ok = true;
        ok = runTimedUnitTest("RTOS Unit test", &UnitTest::runRtosTest, true) && ok;
        ok = runTimedUnitTest("RAM Unit test", &UnitTest::runMemoryTest, true) && ok;
        ok = runTimedUnitTest("Certification Unit test", &UnitTest::runCertificationSuite, false) && ok;
        ok = runTimedUnitTest("Filesystem Unit test", &UnitTest::runFsMainCheck, false) && ok;

        NVSStore::setUInt(NVSKey::Key::LastRequiredUnitTest, 0);

        return ok;
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
uint32_t lastRun = 0;

// ESP-IDF native serial setup (before Arduino initialization)
static void setup_serial(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    esp_log_level_set(NET_TAG, ESP_LOG_INFO);
}

void booting(void)
{
    // ESP-IDF native logging (no Arduino dependency)
    setup_serial();

    // Init Configuration
    if (!Configuration::loadConfigFromFile("/config/require.yml"))
        Log::sys_error(TAG, "Failed to load configuration from file - using defaults");
    else
    {
        Log::sys_info(TAG, "Configuration loaded from file successfully");
        Log::sys_info(TAG, "Config entries loaded: " + String(Configuration::getConfigEntryCount()));

        delay(10);

        for (const auto &entry : Configuration::getConfigEntries())
        {
            Log::sys_info(TAG, "Config entry: " + entry.stmt + " = " + entry.expr);
            delay(10);
        }
    }

    // Read in nvs when last rut was done.
    NVSStore::begin();
    lastRun = NVSStore::getUInt(NVSKey::Key::LastRequiredUnitTest, 0);

    int circleConfig = Configuration::getExprForStmt("rut_circle").toInt();
    std::uint32_t circle = static_cast<std::uint32_t>(defualtRUT);
    if (circleConfig > 0)
    {
        circle = static_cast<std::uint32_t>(circleConfig);
    }

    if (lastRun == kForceRequiredUnitTestsNow || lastRun > circle)
    {
        Log::sys_info("NVS", "Next RUT on next boot");
    }
    else
    {
        Log::sys_info("NVS", "Next RUT in " + String(circle - lastRun) + " init cycles");
    }

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
    Log::sys_info(TAG, "CyberPi cyber address in main: " + String((unsigned long)&cyber, HEX));
    Display::initialize();
    int font_size = 16;
    const wchar_t *titleText = L"HkVLib Firmware";
    Bitmap *bitmap = cyber.create_text(const_cast<wchar_t *>(titleText), 0xffff, font_size);
    cyber.set_bitmap(4, 4, bitmap);
    cyber.render_lcd();

    // Init Unit Test required for init phase (critical tests that must pass for safe operation, otherwise restart)
    // Unit tests for: RTOS, RAM. Needed: WiFi, Bluetooth, peripheral tests, etc. also Security and Data integrity tests.
#if CORRUPT_TEST
    UnitTest::Corrupt::applyBeforeUnitTests();
#endif
    Display::draw_log(cyber, "Running required unit tests...");
    if (!runRequiredUnitTests(circle, lastRun))
    {
        Log::sys_error(TAG, "Critical required unit tests failed - restarting...");
        delay(2000);
        esp_restart();
    }

    // Init Configuration
    if (!Configuration::loadConfigFromFile("/config/config.yml"))
        Log::sys_error(TAG, "Failed to load configuration from file - using defaults");
    else
    {
        Log::sys_info(TAG, "Configuration loaded from file successfully");
        Log::sys_info(TAG, "Config entries loaded: " + String(Configuration::getConfigEntryCount()));

        delay(10);

        for (const auto &entry : Configuration::getConfigEntries())
        {
            Log::sys_info(TAG, "Config entry: " + entry.stmt + " = " + entry.expr);
            delay(10);
        }

        const String &disableId = Configuration::getExprForStmt("disable_id");
        if (!disableId.isEmpty())
            Log::sys_info(TAG, "Config disable_id=" + disableId);
    }

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
    if (g_ssid.isEmpty() || g_password.isEmpty())
    {
        Log::sys_warning(TAG, "WiFi credentials not set in config - using defaults");
        g_ssid = WLAN_SSID;
        g_password = WLAN_PASSWORD;
    }

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
            Log::sys_error(TAG, "WiFi connection failed! Daemon will retry... with default credentials");
            Display::draw_log(cyber, "WiFi connection failed! Daemon will retry... with default credentials");
            g_ssid = WLAN_SSID;
            g_password = WLAN_PASSWORD;
        }
    }

    // TODO: Buggy
    // g_wifi->setDeviceName(DEVICE_NAME);

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

    Display::draw_log(cyber, "Daemons started. Running optional unit tests...");
    // Optional Unit Tests (run in dedicated task to avoid main-task stack overflow)
    /**
     * @brief Unit test that are not nessecary at init time. Unit Test required to run are far higher.
     * p2p protocol, security, OTA, server communication, etc., mqtt, etc.
     */
    UnitTestTaskContext testContext = {false, false};
    TaskHandle_t unitTestTaskHandle = nullptr;
    BaseType_t created = xTaskCreatePinnedToCore(
        runUnitTestsTask,
        "unit_test_task",
        kUnitTestTaskStackSize,
        &testContext,
        tskIDLE_PRIORITY + 1,
        &unitTestTaskHandle,
        xPortGetCoreID());

    Display::draw_log(cyber, "Running optional unit tests...");

    if (created != pdPASS)
    {
        Log::sys_error(TAG, "Failed to create unit test task");
        delay(2000);
        esp_restart();
    }

    if (!optionalTest)
    {
        Log::sys_warning(TAG, "Skip optional test. Stopping unit test task to free resources");
        if (unitTestTaskHandle != nullptr)
        {
            Display::draw_log(cyber, "Skipping optional unit tests...");
            vTaskDelete(unitTestTaskHandle);
            unitTestTaskHandle = nullptr;
        }
        testContext.done = true;
    }
    else
    {
        while (!testContext.done) // add timeout
        {
            delay(10);
        }

        if (unitTestTaskHandle != nullptr)
        {
            vTaskDelete(unitTestTaskHandle);
            unitTestTaskHandle = nullptr;
            Log::sys_info(TAG, "Unit test task finished and was terminated");
        }
    }

    if (optionalTest && !testContext.allPassed)
    {
        Log::sys_warning(TAG, "One or more non-critical unit tests failed");
    }

#if CORRUPT_TEST
    UnitTest::Corrupt::cleanupAfterUnitTests();
#endif

    // Start Health Daemons
    HealthDaemons::startHealthDaemons();

    // TODO: Init Extensive Platform Cyper PI Lib

    // Init done
    Display::draw_log(cyber, "Initialization complete. Starting main loop...");
    delay(1000);
    Display::draw_team(cyber);
    delay(3000);

    // Do Init state Unit Test.
    if (!UnitTest::initUnitTests())
    {
        Log::sys_error(TAG, "Critical Init Unit Test failed - restarting...");
        delay(2000);
        esp_restart();
    }

    Init::initialized();
}

extern "C" void app_main(void)
{
    booting();
    OnlineLock::init();
    Log::sys_info(TAG, "OnlineLock initialized, isLocked=" + String(OnlineLock::isLocked()));
    App::init();
    Log::sys_info(TAG, "App::init() completed - display should be cleared now");

    Display::clear(cyber); // Ensure any init bitmaps are freed to prevent memory issues in main loop
    Display::draw_log(cyber, "Entering main loop...");

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
            continue;    // JUST DON'T USE RETURN
        }

        const String &disableIdValue = Configuration::getExprForStmt("disable_id");
        const bool disableId = disableIdValue.equalsIgnoreCase("true") || disableIdValue == "1";
        if (DEVICE_ID == -1)
        {
            if (disableId)
            {
                Log::sys_info(TAG, "Device ID disabled via config - continuing anyway");
            }
            else
            {
                Log::sys_error(TAG, "Device ID not assigned - waiting for assignment...");
                delay(5000);
                continue;
            }
        }

        Log::sys_info(TAG, "Calling App::update() iteration...");
        App::update();
        Log::sys_info(TAG, "App::update() completed");
        delay(10);
    }
}
