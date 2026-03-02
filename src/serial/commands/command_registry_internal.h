#pragma once

#include <map>

#include "./command_registry.h"
#include "../../nvs/nvs.h"

namespace SerialCommandRegistry
{
    namespace Internal
    {
        constexpr const char *kTag = "SERIAL";
        constexpr const char *kConfigMount = "/config";

        struct StringLess
        {
            bool operator()(const String &lhs, const String &rhs) const
            {
                return lhs.compareTo(rhs) < 0;
            }
        };

        using CommandHandler = bool (*)(const String &args);

        struct CommandDescriptor
        {
            String command;
            String description;
            String usage;
            bool acceptsArgs;
            CommandHandler handler;
        };

        using CommandMap = std::map<String, CommandDescriptor, StringLess>;

        String normalizeCommand(const String &input);
        bool printNotImplemented(const char *commandName);
        bool resolveNvsKey(const String &keyNameRaw, NVSKey::Key &keyOut);
        String toAbsoluteConfigPath(const String &inputPath);
        void printConfigEntries();

        bool handleHelp(const String &args);
        bool handleWifiStatus(const String &args);
        bool handleReboot(const String &args);
        bool handleRtosTasks(const String &args);
        bool handleDisplayPing(const String &args);
        bool handleDaemonNotify(const String &args);
        bool handleRut(const String &args);
        bool handleVersion(const String &args);
        bool handleUptime(const String &args);
        bool handleHeapStatus(const String &args);
        bool handleBootReason(const String &args);
        bool handleWifiScan(const String &args);
        bool handleWifiConnect(const String &args);
        bool handleWifiDisconnect(const String &args);
        bool handleWifiIp(const String &args);
        bool handleWifiReconnect(const String &args);
        bool handleDaemonList(const String &args);
        bool handleFsInfo(const String &args);
        bool handleFsLs(const String &args);
        bool handleFsCat(const String &args);
        bool handleFsRm(const String &args);
        bool handleFsTest(const String &args);
        bool handleNvsList(const String &args);
        bool handleNvsGet(const String &args);
        bool handleNvsSet(const String &args);
        bool handleConfigReload(const String &args);
        bool handleConfigShow(const String &args);
        bool handleUtRequired(const String &args);
        bool handleUtOptional(const String &args);
        bool handleUtFs(const String &args);
        bool handleUtRtos(const String &args);
        bool handleUtStatus(const String &args);
        bool handleDisplayClear(const String &args);
        bool handleDisplayText(const String &args);
        bool handleDisplayBrightness(const String &args);
        bool handleNotImplAppStart(const String &args);
        bool handleNotImplAppStop(const String &args);
        bool handleNotImplAppRestart(const String &args);
        bool handleNotImplAppState(const String &args);
        bool handleNotImplAppUpload(const String &args);
        bool handleNotImplNetPing(const String &args);
        bool handleNotImplNetDns(const String &args);
        bool handleNotImplNetTime(const String &args);
        bool handleNotImplNetReq(const String &args);
        bool handleNotImplNetStats(const String &args);
        bool handleNotImplDaemonStart(const String &args);
        bool handleNotImplDaemonStop(const String &args);
        bool handleNotImplDaemonRestart(const String &args);
        bool handleNotImplDaemonWatchdog(const String &args);
        bool handleNotImplSoundBeep(const String &args);
        bool handleNotImplSensorStatus(const String &args);

        void registerCommand(CommandMap &commands, const char *command, const char *description, const char *usage, bool acceptsArgs, CommandHandler handler);
        void registerAllCommands(CommandMap &commands);
        CommandMap &getCommandMap();
    }
}
