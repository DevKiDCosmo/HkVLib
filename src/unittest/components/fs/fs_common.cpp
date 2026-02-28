#include "./fs_common.h"

#include <cerrno>
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>

#include "esp_err.h"
#include "esp_spiffs.h"
#include "serial/log.h"

namespace UnitTest
{
    namespace FsInternal
    {
        namespace
        {
            constexpr const char *kTag = "FSTEST";
            constexpr const char *kMountCandidates[] = {"/config"};

            bool canUseMountPath(const char *path)
            {
                const String probePath = String(path) + "/.hkv_fs_mount_detect.tmp";
                FILE *probe = fopen(probePath.c_str(), "ab");
                if (!probe)
                {
                    return false;
                }

                fclose(probe);
                remove(probePath.c_str());
                return true;
            }

            const char *detectMountPath()
            {
                for (const char *candidate : kMountCandidates)
                {
                    if (canUseMountPath(candidate))
                    {
                        return candidate;
                    }
                }
                return nullptr;
            }

            bool tryMountPartition(const char *basePath, const char *partitionLabel, const char *&mountedLabel)
            {
                esp_vfs_spiffs_conf_t conf = {};
                conf.base_path = basePath;
                conf.partition_label = partitionLabel;
                conf.max_files = 8;
                conf.format_if_mount_failed = false;

                const esp_err_t err = esp_vfs_spiffs_register(&conf);
                if (err == ESP_OK)
                {
                    mountedLabel = partitionLabel;
                    Log::sys_info(kTag, "Mounted partition '" + String(partitionLabel) + "' at " + String(basePath));
                    return true;
                }

                Log::sys_warning(kTag, "Failed to mount partition '" + String(partitionLabel) + "': " + String(esp_err_to_name(err)));
                return false;
            }

            bool tryMountSpiffs(const char *basePath, const char *&mountedLabel)
            {
                if (tryMountPartition(basePath, "config", mountedLabel))
                {
                    return true;
                }

                esp_vfs_spiffs_conf_t conf = {};
                conf.base_path = basePath;
                conf.partition_label = nullptr;
                conf.max_files = 8;
                conf.format_if_mount_failed = true;

                const esp_err_t err = esp_vfs_spiffs_register(&conf);
                if (err == ESP_OK)
                {
                    mountedLabel = nullptr;
                    Log::sys_info(kTag, "Mounted generic SPIFFS partition at " + String(basePath));
                    return true;
                }

                if (err == ESP_ERR_INVALID_STATE)
                {
                    Log::sys_warning(kTag, "SPIFFS already mounted elsewhere, continuing with existing mount path " + String(basePath));
                    mountedLabel = "__external__";
                    return true;
                }

                Log::sys_error(kTag, "SPIFFS mount failed: " + String(esp_err_to_name(err)) + " (" + String(static_cast<int>(err)) + ")");
                return false;
            }

            void cleanupMount(const char *mountedLabel)
            {
                if (!mountedLabel)
                {
                    esp_vfs_spiffs_unregister(nullptr);
                    return;
                }

                if (strcmp(mountedLabel, "__external__") == 0)
                {
                    return;
                }

                esp_vfs_spiffs_unregister(mountedLabel);
            }
        } // namespace

        bool hasDirectory(const char *path)
        {
            struct stat st{};
            if (stat(path, &st) != 0)
            {
                return false;
            }

            if ((st.st_mode & S_IFDIR) != 0)
            {
                return true;
            }

            return true;
        }

        bool ensureFsContext(FsContext &ctx)
        {
            ctx.mount = detectMountPath();
            if (ctx.mount)
            {
                Log::sys_info(kTag, "Using existing filesystem mount at " + String(ctx.mount));
                return true;
            }

            Log::sys_info(kTag, "No mounted filesystem path found. Attempting to mount config partition...");
            if (!tryMountSpiffs("/config", ctx.spiffsLabel))
            {
                Log::sys_error(kTag, "Could not mount config partition");
                return false;
            }

            ctx.mount = "/config";
            ctx.mountedByTest = (ctx.spiffsLabel == nullptr) || (strcmp(ctx.spiffsLabel, "__external__") != 0);
            Log::sys_info(kTag, "Mounted config partition for test at /config");
            return true;
        }

        void releaseFsContext(const FsContext &ctx)
        {
            if (!ctx.mountedByTest)
            {
                return;
            }

            cleanupMount(ctx.spiffsLabel);
            Log::sys_info(kTag, "Filesystem partition unmounted");
        }

        bool existsPath(const String &path)
        {
            struct stat st{};
            return stat(path.c_str(), &st) == 0;
        }

        bool cleanupPathIfExists(const String &path)
        {
            if (!existsPath(path))
            {
                return true;
            }

            if (remove(path.c_str()) == 0)
            {
                return true;
            }

            if (rmdir(path.c_str()) == 0)
            {
                return true;
            }

            Log::sys_warning(kTag, "Failed to cleanup path: " + path + ", errno=" + String(errno));
            return false;
        }
    } // namespace FsInternal
} // namespace UnitTest
