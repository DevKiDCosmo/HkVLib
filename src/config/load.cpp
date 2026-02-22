#include "esp_spiffs.h"
#include <fstream>
#include "../serial/log.h"
#include "config.h"

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

bool Configuration::loadConfigFromFile(const char *filePath = "/config/config.yml")
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

    std::string line;
    while (std::getline(file, line))
    {
        // Parse YAML config lines
        // Example: "wlan_ssid: LGS_intern"
        size_t delimiter = line.find(':');
        if (delimiter != std::string::npos)
        {
            std::string key = line.substr(0, delimiter);
            std::string value = line.substr(delimiter + 2); // skip ": "

            if (key == "wlan_ssid")
            {
                // Store WLAN SSID
                Log::sys_info("CONFIG", "Found WLAN SSID: " + String(value.c_str()));
            }
            else if (key == "wlan_pass")
            {
                // Store WLAN Password
                Log::sys_info("CONFIG", "Found WLAN Password: " + String(value.c_str()));
            }
            // Add more parsing as needed
        }
    }

    file.close();
    return true;
}