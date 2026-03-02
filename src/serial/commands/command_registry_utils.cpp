#include "./command_registry_internal.h"

#include <cstdio>

#include "../../config/config.h"

namespace SerialCommandRegistry
{
    namespace Internal
    {
        String normalizeCommand(const String &input)
        {
            String out = input;
            out.trim();
            out.toLowerCase();
            return out;
        }

        bool printNotImplemented(const char *commandName)
        {
            Serial.printf("%s: not implemented yet.\n", commandName);
            return true;
        }

        bool resolveNvsKey(const String &keyNameRaw, NVSKey::Key &keyOut)
        {
            String keyName = normalizeCommand(keyNameRaw);
            if (keyName == "serversha256" || keyName == "server_sha256")
            {
                keyOut = NVSKey::Key::ServerSha256;
                return true;
            }
            if (keyName == "configsha256" || keyName == "config_sha256")
            {
                keyOut = NVSKey::Key::ConfigSha256;
                return true;
            }
            if (keyName == "devicepassword" || keyName == "device_pw")
            {
                keyOut = NVSKey::Key::DevicePassword;
                return true;
            }
            if (keyName == "unittestdone" || keyName == "ut_done")
            {
                keyOut = NVSKey::Key::UnitTestDone;
                return true;
            }
            if (keyName == "lastunittestms" || keyName == "ut_last_ms")
            {
                keyOut = NVSKey::Key::LastUnitTestMs;
                return true;
            }
            if (keyName == "lastrequiredunittest" || keyName == "lastrut")
            {
                keyOut = NVSKey::Key::LastRequiredUnitTest;
                return true;
            }

            return false;
        }

        String toAbsoluteConfigPath(const String &inputPath)
        {
            String path = inputPath;
            path.trim();

            if (path.isEmpty())
            {
                return String(kConfigMount);
            }

            if (path.startsWith("/config"))
            {
                return path;
            }

            if (path.startsWith("/"))
            {
                return String(kConfigMount) + path;
            }

            return String(kConfigMount) + "/" + path;
        }

        void printConfigEntries()
        {
            const auto &entries = Configuration::getConfigEntries();
            if (entries.empty())
            {
                Serial.println("config: no entries loaded");
                return;
            }

            Serial.printf("config: %d entries\n", Configuration::getConfigEntryCount());
            for (const auto &entry : entries)
            {
                Serial.printf("- %s = %s\n", entry.stmt.c_str(), entry.expr.c_str());
            }
        }

        void registerCommand(CommandMap &commands, const char *command, const char *description, const char *usage, bool acceptsArgs, CommandHandler handler)
        {
            CommandDescriptor descriptor;
            descriptor.command = command;
            descriptor.description = description;
            descriptor.usage = usage;
            descriptor.acceptsArgs = acceptsArgs;
            descriptor.handler = handler;

            commands[String(command)] = descriptor;
        }
    }
}
