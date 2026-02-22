#include "esp_spiffs.h"
#include <fstream>
#include <vector>
#include <cctype>
#include <cstring>
#include "../serial/log.h"
#include "config.h"

static std::vector<Configuration::ConfigEntry> s_configEntries;

static std::string trim(const std::string &input)
{
    size_t start = 0;
    while (start < input.size() && std::isspace(static_cast<unsigned char>(input[start])))
    {
        ++start;
    }

    if (start == input.size())
    {
        return "";
    }

    size_t end = input.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(input[end])))
    {
        --end;
    }

    return input.substr(start, end - start + 1);
}

static std::string stripWrappingQuotes(const std::string &input)
{
    if (input.size() >= 2)
    {
        const char first = input.front();
        const char last = input.back();
        if ((first == '"' && last == '"') || (first == '\'' && last == '\''))
        {
            return input.substr(1, input.size() - 2);
        }
    }

    return input;
}

static bool mountConfigPartition()
{
    esp_vfs_spiffs_conf_t conf = {};
    conf.base_path = "/config";
    conf.partition_label = "config";
    conf.max_files = 4;
    conf.format_if_mount_failed = false;

    esp_err_t err = esp_vfs_spiffs_register(&conf);
    if (err == ESP_OK)
    {
        Log::sys_info("CONFIG", "Config partition mounted at /config");
        return true;
    }

    Log::sys_warning("CONFIG", "Failed to mount config partition: " + String(esp_err_to_name(err)));
    return false;
}

bool Configuration::loadConfigFromFile(const char *filePath)
{
    // Try to mount config partition if not already mounted
    static bool partitionMounted = false;
    if (!partitionMounted)
    {
        partitionMounted = mountConfigPartition();
    }

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        Log::sys_error("CONFIG", "Could not open config file: " + String(filePath));
        return false;
    }

    s_configEntries.clear();

    std::string line;
    while (std::getline(file, line))
    {
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos)
        {
            line = line.substr(0, commentPos);
        }

        line = trim(line);
        if (line.empty())
        {
            continue;
        }

        size_t delimiter = line.find(':');
        if (delimiter != std::string::npos)
        {
            std::string key = trim(line.substr(0, delimiter));
            std::string value = trim(line.substr(delimiter + 1));
            value = stripWrappingQuotes(value);

            if (key.empty())
            {
                continue;
            }

            s_configEntries.push_back({String(key.c_str()), String(value.c_str())});

            if (key == "wlan_ssid")
            {
                // Store WLAN SSID
                Log::sys_info("CONFIG", "Found WLAN SSID: " + String(value.c_str()));
                g_ssid = value.c_str();
            }
            else if (key == "wlan_pass")
            {
                // Store WLAN Password
                Log::sys_info("CONFIG", "Found WLAN Password: " + String(value.c_str()));
                g_password = value.c_str();
            }
            // Add more parsing as needed
        }
    }

    file.close();
    return true;
}

const std::vector<Configuration::ConfigEntry> &Configuration::getConfigEntries()
{
    return s_configEntries;
}

int Configuration::getConfigEntryCount()
{
    return static_cast<int>(s_configEntries.size());
}

const String &Configuration::getExprForStmt(const char *stmt)
{
    static String emptyExpr = "";
    if (stmt == nullptr)
    {
        return emptyExpr;
    }

    for (auto &entry : s_configEntries)
    {
        if (std::strcmp(entry.stmt.c_str(), stmt) == 0)
        {
            return entry.expr;
        }
    }

    return emptyExpr;
}

bool Configuration::hasStmt(const char *stmt)
{
    return getExprForStmt(stmt) != "";
}