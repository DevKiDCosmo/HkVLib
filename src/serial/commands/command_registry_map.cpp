#include "./command_registry_internal.h"

namespace SerialCommandRegistry
{
    namespace Internal
    {
        void registerAllCommands(CommandMap &commands)
        {
            registerCommand(commands, "help", "List all serial commands", "help", false, &handleHelp);
            registerCommand(commands, "version", "Show firmware version/build/date", "version", false, &handleVersion);
            registerCommand(commands, "uptime", "Show uptime in milliseconds", "uptime", false, &handleUptime);
            registerCommand(commands, "heap status", "Show heap statistics", "heap status", false, &handleHeapStatus);
            registerCommand(commands, "boot reason", "Show reset reason", "boot reason", false, &handleBootReason);

            registerCommand(commands, "wifi status", "Show WiFi status and IP", "wifi status", false, &handleWifiStatus);
            registerCommand(commands, "wifi scan", "Scan visible WiFi networks", "wifi scan", false, &handleWifiScan);
            registerCommand(commands, "wifi connect", "Connect to WiFi with SSID/password", "wifi connect <ssid> <pw>", true, &handleWifiConnect);
            registerCommand(commands, "wifi disconnect", "Disconnect WiFi", "wifi disconnect", false, &handleWifiDisconnect);
            registerCommand(commands, "wifi ip", "Show current WiFi IP", "wifi ip", false, &handleWifiIp);
            registerCommand(commands, "wifi reconnect", "Reconnect with cached credentials", "wifi reconnect", false, &handleWifiReconnect);

            registerCommand(commands, "reboot", "Restart device immediately", "reboot", false, &handleReboot);

            registerCommand(commands, "rtos tasks", "Show RTOS task table", "rtos tasks", false, &handleRtosTasks);
            registerCommand(commands, "daemon status", "Alias for rtos tasks", "daemon status", false, &handleRtosTasks);
            registerCommand(commands, "daemon list", "List daemon/task states", "daemon list", false, &handleDaemonList);
            registerCommand(commands, "daemon start", "Start daemon by name", "daemon start <name>", true, &handleNotImplDaemonStart);
            registerCommand(commands, "daemon stop", "Stop daemon by name", "daemon stop <name>", true, &handleNotImplDaemonStop);
            registerCommand(commands, "daemon restart", "Restart daemon by name", "daemon restart <name>", true, &handleNotImplDaemonRestart);
            registerCommand(commands, "daemon watchdog", "Show daemon watchdog status", "daemon watchdog", false, &handleNotImplDaemonWatchdog);

            registerCommand(commands, "display ping", "Render display diagnostics", "display ping", false, &handleDisplayPing);
            registerCommand(commands, "display test", "Alias for display ping", "display test", false, &handleDisplayPing);
            registerCommand(commands, "display clear", "Clear display", "display clear", false, &handleDisplayClear);
            registerCommand(commands, "display text", "Render text on display", "display text <msg>", true, &handleDisplayText);
            registerCommand(commands, "display brightness", "Set display brightness", "display brightness <0-100>", true, &handleDisplayBrightness);

            registerCommand(commands, "daemon notify", "Watch task until it is running again", "daemon notify <taskname>", true, &handleDaemonNotify);
            registerCommand(commands, "rut", "Force required unit tests on next boot", "rut", false, &handleRut);

            registerCommand(commands, "app start", "Start app runtime", "app start", false, &handleNotImplAppStart);
            registerCommand(commands, "app stop", "Stop app runtime", "app stop", false, &handleNotImplAppStop);
            registerCommand(commands, "app restart", "Restart app runtime", "app restart", false, &handleNotImplAppRestart);
            registerCommand(commands, "app state", "Show app state", "app state", false, &handleNotImplAppState);
            registerCommand(commands, "app upload", "Enter upload-safe mode", "app upload", false, &handleNotImplAppUpload);

            registerCommand(commands, "net ping", "Ping remote host", "net ping <host>", true, &handleNotImplNetPing);
            registerCommand(commands, "net dns", "Resolve host via DNS", "net dns <host>", true, &handleNotImplNetDns);
            registerCommand(commands, "net time", "Show network/system time", "net time", false, &handleNotImplNetTime);
            registerCommand(commands, "net req", "Perform test HTTP request", "net req <url>", true, &handleNotImplNetReq);
            registerCommand(commands, "net stats", "Show network statistics", "net stats", false, &handleNotImplNetStats);

            registerCommand(commands, "fs info", "Show filesystem usage", "fs info", false, &handleFsInfo);
            registerCommand(commands, "fs ls", "List files in path", "fs ls [path]", true, &handleFsLs);
            registerCommand(commands, "fs cat", "Print file content", "fs cat <file>", true, &handleFsCat);
            registerCommand(commands, "fs rm", "Remove file", "fs rm <file>", true, &handleFsRm);
            registerCommand(commands, "fs test", "Run filesystem unit tests", "fs test", false, &handleFsTest);

            registerCommand(commands, "nvs list", "List known NVS keys", "nvs list", false, &handleNvsList);
            registerCommand(commands, "nvs get", "Read NVS key", "nvs get <key>", true, &handleNvsGet);
            registerCommand(commands, "nvs set", "Write NVS key", "nvs set <key> <value>", true, &handleNvsSet);
            registerCommand(commands, "config reload", "Reload config file", "config reload", false, &handleConfigReload);
            registerCommand(commands, "config show", "Print loaded config entries", "config show", false, &handleConfigShow);

            registerCommand(commands, "ut required", "Force required UT next boot", "ut required", false, &handleUtRequired);
            registerCommand(commands, "ut optional", "Run optional unit tests now", "ut optional", false, &handleUtOptional);
            registerCommand(commands, "ut fs", "Run filesystem unit tests now", "ut fs", false, &handleUtFs);
            registerCommand(commands, "ut rtos", "Run RTOS unit tests now", "ut rtos", false, &handleUtRtos);
            registerCommand(commands, "ut status", "Show unit test status", "ut status", false, &handleUtStatus);

            registerCommand(commands, "sound beep", "Beep sound test", "sound beep", false, &handleNotImplSoundBeep);
            registerCommand(commands, "sensor status", "Show sensor status", "sensor status", false, &handleNotImplSensorStatus);
        }

        CommandMap &getCommandMap()
        {
            static CommandMap commands;
            static bool initialized = false;

            if (!initialized)
            {
                registerAllCommands(commands);
                initialized = true;
            }

            return commands;
        }
    }
}
