#include "./command_registry_internal.h"

#include <dirent.h>
#include <cstdio>
#include <cstring>

#include "WiFi.h"
#include "../../display/display.h"
#include "../../nvs/nvs.h"
#include "../../serial/log.h"
#include "../../unittest/components/RTOS/RTOS.h"
#include "../../unittest/components/fs/fs.h"
#include "../../unittest/math.h"
#include "../../unittest/psram.h"
#include "./debug.h"

#include "esp_heap_caps.h"
#include "esp_spiffs.h"
#include "esp_system.h"

namespace SerialCommandRegistry
{
    namespace Internal
    {
        bool handleHelp(const String &args)
        {
            (void)args;
            SerialCommandRegistry::printHelp();
            return true;
        }

        bool handleWifiStatus(const String &args)
        {
            (void)args;

            if (g_wifi != nullptr)
            {
                Log::sys_info(kTag, "WiFi Status: " + String(g_wifi->isConnected() ? "Connected" : "Disconnected") + ", IP: " + (g_wifi->isConnected() ? g_wifi->getLocalIP() : "N/A"));
                return true;
            }

            Log::sys_warning(kTag, "WiFi not initialized");
            return true;
        }

        bool handleReboot(const String &args)
        {
            (void)args;
            Log::sys_info(kTag, "Rebooting system...");
            esp_restart();
            return true;
        }

        bool handleRtosTasks(const String &args)
        {
            (void)args;
            SerialDebugCommands::RTOSBgTask();
            return true;
        }

        bool handleDisplayPing(const String &args)
        {
            (void)args;
            SerialDebugCommands::DisplayPing();
            return true;
        }

        bool handleDaemonNotify(const String &args)
        {
            String taskName = args;
            taskName.trim();
            SerialDebugCommands::DaemonNotify(taskName);
            return true;
        }

        bool handleRut(const String &args)
        {
            (void)args;
            NVSStore::setUInt(NVSKey::Key::LastRequiredUnitTest, 0xFFFFFFFFu);
            Log::sys_info(kTag, "Required Unit Tests will run on next boot");
            return true;
        }

        bool handleVersion(const String &args)
        {
            (void)args;
            Serial.printf("version: %s | build: %s | date: %s\n", VERSION, BUILD, DATE);
            return true;
        }

        bool handleUptime(const String &args)
        {
            (void)args;
            const unsigned long ms = millis();
            Serial.printf("uptime: %lu ms\n", ms);
            return true;
        }

        bool handleHeapStatus(const String &args)
        {
            (void)args;
            Serial.printf("heap: free=%u internal=%u psram=%u\n",
                          static_cast<unsigned>(esp_get_free_heap_size()),
                          static_cast<unsigned>(heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)),
                          static_cast<unsigned>(heap_caps_get_free_size(MALLOC_CAP_SPIRAM)));
            return true;
        }

        const char *resetReasonToString(esp_reset_reason_t reason)
        {
            switch (reason)
            {
            case ESP_RST_UNKNOWN:
                return "unknown";
            case ESP_RST_POWERON:
                return "poweron";
            case ESP_RST_EXT:
                return "external";
            case ESP_RST_SW:
                return "software";
            case ESP_RST_PANIC:
                return "panic";
            case ESP_RST_INT_WDT:
                return "interrupt watchdog";
            case ESP_RST_TASK_WDT:
                return "task watchdog";
            case ESP_RST_WDT:
                return "other watchdog";
            case ESP_RST_DEEPSLEEP:
                return "deepsleep";
            case ESP_RST_BROWNOUT:
                return "brownout";
            case ESP_RST_SDIO:
                return "sdio";
            default:
                return "n/a";
            }
        }

        bool handleBootReason(const String &args)
        {
            (void)args;
            const esp_reset_reason_t reason = esp_reset_reason();
            Serial.printf("boot reason: %s (%d)\n", resetReasonToString(reason), static_cast<int>(reason));
            return true;
        }

        bool handleWifiScan(const String &args)
        {
            (void)args;
            const int count = WiFi.scanNetworks();
            if (count < 0)
            {
                Serial.println("wifi scan failed");
                return true;
            }

            Serial.printf("wifi scan: %d network(s)\n", count);
            for (int i = 0; i < count; ++i)
            {
                Serial.printf("- %s (RSSI %d)\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
            }
            return true;
        }

        bool handleWifiConnect(const String &args)
        {
            if (g_wifi == nullptr)
            {
                Serial.println("wifi connect: WiFi object not initialized");
                return true;
            }

            String parsed = args;
            parsed.trim();
            const int sep = parsed.indexOf(' ');
            if (sep <= 0)
            {
                Serial.println("usage: wifi connect <ssid> <pw>");
                return true;
            }

            const String ssid = parsed.substring(0, sep);
            String password = parsed.substring(sep + 1);
            password.trim();
            if (password.isEmpty())
            {
                Serial.println("usage: wifi connect <ssid> <pw>");
                return true;
            }

            g_ssid = ssid;
            g_password = password;
            const bool ok = g_wifi->connect(g_ssid, g_password);
            Serial.printf("wifi connect: %s\n", ok ? "ok" : "failed");
            return true;
        }

        bool handleWifiDisconnect(const String &args)
        {
            (void)args;
            if (g_wifi == nullptr)
            {
                Serial.println("wifi disconnect: WiFi object not initialized");
                return true;
            }

            g_wifi->disconnect();
            Serial.println("wifi disconnect: done");
            return true;
        }

        bool handleWifiIp(const String &args)
        {
            (void)args;
            if (g_wifi == nullptr)
            {
                Serial.println("wifi ip: WiFi object not initialized");
                return true;
            }

            Serial.printf("wifi ip: %s\n", g_wifi->isConnected() ? g_wifi->getLocalIP().c_str() : "N/A");
            return true;
        }

        bool handleWifiReconnect(const String &args)
        {
            (void)args;
            if (g_wifi == nullptr)
            {
                Serial.println("wifi reconnect: WiFi object not initialized");
                return true;
            }

            if (g_ssid.isEmpty())
            {
                Serial.println("wifi reconnect: no cached credentials");
                return true;
            }

            const bool ok = g_wifi->connect(g_ssid, g_password);
            Serial.printf("wifi reconnect: %s\n", ok ? "ok" : "failed");
            return true;
        }

        bool handleDaemonList(const String &args)
        {
            (void)args;
            SerialDebugCommands::RTOSBgTask();
            return true;
        }

        bool handleFsInfo(const String &args)
        {
            (void)args;

            std::size_t total = 0;
            std::size_t used = 0;
            const esp_err_t err = esp_spiffs_info("config", &total, &used);
            if (err != ESP_OK)
            {
                Serial.printf("fs info failed: %s\n", esp_err_to_name(err));
                return true;
            }

            Serial.printf("fs info: mount=%s total=%u used=%u free=%u\n",
                          kConfigMount,
                          static_cast<unsigned>(total),
                          static_cast<unsigned>(used),
                          static_cast<unsigned>(total - used));
            return true;
        }

        bool handleFsLs(const String &args)
        {
            const String path = toAbsoluteConfigPath(args.isEmpty() ? String(kConfigMount) : args);
            DIR *dir = opendir(path.c_str());
            if (!dir)
            {
                Serial.printf("fs ls failed: %s\n", path.c_str());
                return true;
            }

            Serial.printf("fs ls: %s\n", path.c_str());
            while (dirent *entry = readdir(dir))
            {
                Serial.printf("- %s\n", entry->d_name);
            }
            closedir(dir);
            return true;
        }

        bool handleFsCat(const String &args)
        {
            const String path = toAbsoluteConfigPath(args);
            FILE *file = fopen(path.c_str(), "rb");
            if (!file)
            {
                Serial.printf("fs cat failed: %s\n", path.c_str());
                return true;
            }

            Serial.printf("fs cat: %s\n", path.c_str());
            char buffer[96];
            while (true)
            {
                const std::size_t n = fread(buffer, 1u, sizeof(buffer) - 1u, file);
                if (n == 0u)
                {
                    break;
                }
                buffer[n] = '\0';
                Serial.print(buffer);
            }
            Serial.println();
            fclose(file);
            return true;
        }

        bool handleFsRm(const String &args)
        {
            const String path = toAbsoluteConfigPath(args);
            const int rc = remove(path.c_str());
            Serial.printf("fs rm: %s (%s)\n", path.c_str(), (rc == 0) ? "ok" : "failed");
            return true;
        }

        bool handleFsTest(const String &args)
        {
            (void)args;
            const bool ok = UnitTest::runFsMainCheck();
            Serial.printf("fs test: %s\n", ok ? "ok" : "failed");
            return true;
        }

        bool handleNvsList(const String &args)
        {
            (void)args;
            Serial.println("nvs keys:");
            Serial.println("- server_sha256");
            Serial.println("- config_sha256");
            Serial.println("- device_pw");
            Serial.println("- ut_done");
            Serial.println("- ut_last_ms");
            Serial.println("- lastRUT");
            return true;
        }

        bool handleNvsGet(const String &args)
        {
            String key = args;
            key.trim();
            if (key.isEmpty())
            {
                Serial.println("usage: nvs get <key>");
                return true;
            }

            NVSKey::Key mapped;
            if (!resolveNvsKey(key, mapped))
            {
                Serial.printf("nvs get: unknown key '%s'\n", key.c_str());
                return true;
            }

            const String asString = NVSStore::getString(mapped, "");
            const std::uint32_t asUInt = NVSStore::getUInt(mapped, 0u);
            const bool asBool = NVSStore::getBool(mapped, false);

            Serial.printf("nvs get: key=%s str='%s' uint=%u bool=%s\n",
                          key.c_str(),
                          asString.c_str(),
                          static_cast<unsigned>(asUInt),
                          asBool ? "true" : "false");
            return true;
        }

        bool handleNvsSet(const String &args)
        {
            String parsed = args;
            parsed.trim();
            const int sep = parsed.indexOf(' ');
            if (sep <= 0)
            {
                Serial.println("usage: nvs set <key> <value>");
                return true;
            }

            const String key = parsed.substring(0, sep);
            String value = parsed.substring(sep + 1);
            value.trim();

            NVSKey::Key mapped;
            if (!resolveNvsKey(key, mapped))
            {
                Serial.printf("nvs set: unknown key '%s'\n", key.c_str());
                return true;
            }

            bool ok = false;
            if (value.equalsIgnoreCase("true") || value.equalsIgnoreCase("false"))
            {
                ok = NVSStore::setBool(mapped, value.equalsIgnoreCase("true"));
            }
            else
            {
                char *end = nullptr;
                const unsigned long numeric = std::strtoul(value.c_str(), &end, 10);
                if (end != nullptr && *end == '\0')
                {
                    ok = NVSStore::setUInt(mapped, static_cast<std::uint32_t>(numeric));
                }
                else
                {
                    ok = NVSStore::setString(mapped, value);
                }
            }

            Serial.printf("nvs set: %s\n", ok ? "ok" : "failed");
            return true;
        }

        bool handleConfigReload(const String &args)
        {
            (void)args;
            const bool ok = Configuration::loadConfigFromFile("/config/config.yml");
            Serial.printf("config reload: %s\n", ok ? "ok" : "failed");
            return true;
        }

        bool handleConfigShow(const String &args)
        {
            (void)args;
            printConfigEntries();
            return true;
        }

        bool handleUtRequired(const String &args)
        {
            return handleRut(args);
        }

        bool handleUtOptional(const String &args)
        {
            (void)args;
            bool ok = true;
            ok = UnitTest::runMathTest() && ok;
            ok = UnitTest::runPsramTest() && ok;
            Serial.printf("ut optional: %s\n", ok ? "ok" : "failed");
            return true;
        }

        bool handleUtFs(const String &args)
        {
            (void)args;
            const bool ok = UnitTest::runFsMainCheck();
            Serial.printf("ut fs: %s\n", ok ? "ok" : "failed");
            return true;
        }

        bool handleUtRtos(const String &args)
        {
            (void)args;
            const bool ok = UnitTest::runRtosTest();
            Serial.printf("ut rtos: %s\n", ok ? "ok" : "failed");
            return true;
        }

        bool handleUtStatus(const String &args)
        {
            (void)args;
            const std::uint32_t lastRequired = NVSStore::getUInt(NVSKey::Key::LastRequiredUnitTest, 0);
            Serial.printf("ut status: lastRUT=%u\n", static_cast<unsigned>(lastRequired));
            return true;
        }

        bool handleDisplayClear(const String &args)
        {
            (void)args;
            Display::clear(cyber);
            Serial.println("display clear: ok");
            return true;
        }

        bool handleDisplayText(const String &args)
        {
            String msg = args;
            msg.trim();
            if (msg.isEmpty())
            {
                Serial.println("usage: display text <msg>");
                return true;
            }

            Display::draw_log(cyber, msg);
            Serial.println("display text: ok");
            return true;
        }

        bool handleDisplayBrightness(const String &args)
        {
            (void)args;
            return printNotImplemented("display brightness");
        }

        bool handleNotImplAppStart(const String &args)
        {
            (void)args;
            return printNotImplemented("app start");
        }

        bool handleNotImplAppStop(const String &args)
        {
            (void)args;
            return printNotImplemented("app stop");
        }

        bool handleNotImplAppRestart(const String &args)
        {
            (void)args;
            return printNotImplemented("app restart");
        }

        bool handleNotImplAppState(const String &args)
        {
            (void)args;
            return printNotImplemented("app state");
        }

        bool handleNotImplAppUpload(const String &args)
        {
            (void)args;
            return printNotImplemented("app upload");
        }

        bool handleNotImplNetPing(const String &args)
        {
            (void)args;
            return printNotImplemented("net ping");
        }

        bool handleNotImplNetDns(const String &args)
        {
            (void)args;
            return printNotImplemented("net dns");
        }

        bool handleNotImplNetTime(const String &args)
        {
            (void)args;
            return printNotImplemented("net time");
        }

        bool handleNotImplNetReq(const String &args)
        {
            (void)args;
            return printNotImplemented("net req");
        }

        bool handleNotImplNetStats(const String &args)
        {
            (void)args;
            return printNotImplemented("net stats");
        }

        bool handleNotImplDaemonStart(const String &args)
        {
            (void)args;
            return printNotImplemented("daemon start");
        }

        bool handleNotImplDaemonStop(const String &args)
        {
            (void)args;
            return printNotImplemented("daemon stop");
        }

        bool handleNotImplDaemonRestart(const String &args)
        {
            (void)args;
            return printNotImplemented("daemon restart");
        }

        bool handleNotImplDaemonWatchdog(const String &args)
        {
            (void)args;
            return printNotImplemented("daemon watchdog");
        }

        bool handleNotImplSoundBeep(const String &args)
        {
            (void)args;
            return printNotImplemented("sound beep");
        }

        bool handleNotImplSensorStatus(const String &args)
        {
            (void)args;
            return printNotImplemented("sensor status");
        }
    }
}
