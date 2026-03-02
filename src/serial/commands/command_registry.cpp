#include "./command_registry_internal.h"

namespace SerialCommandRegistry
{
    bool dispatch(const String &input)
    {
        String normalized = Internal::normalizeCommand(input);
        if (normalized.isEmpty())
        {
            return true;
        }

        Internal::CommandMap &commands = Internal::getCommandMap();

        auto exactIt = commands.find(normalized);
        if (exactIt != commands.end())
        {
            return exactIt->second.handler("");
        }

        for (const auto &entry : commands)
        {
            const Internal::CommandDescriptor &descriptor = entry.second;
            if (!descriptor.acceptsArgs)
            {
                continue;
            }

            const String prefix = descriptor.command + " ";
            if (normalized.equalsIgnoreCase(descriptor.command))
            {
                return descriptor.handler("");
            }

            if (normalized.startsWith(prefix))
            {
                String args = normalized.substring(prefix.length());
                args.trim();
                return descriptor.handler(args);
            }
        }

        return false;
    }

    void printHelp()
    {
        Internal::CommandMap &commands = Internal::getCommandMap();

        Serial.println("\n=== Serial Command Help ===");
        for (const auto &entry : commands)
        {
            const Internal::CommandDescriptor &descriptor = entry.second;
            Serial.printf("- %-20s : %s\n", descriptor.command.c_str(), descriptor.description.c_str());
            Serial.printf("  usage: %s\n", descriptor.usage.c_str());
        }
    }
}
