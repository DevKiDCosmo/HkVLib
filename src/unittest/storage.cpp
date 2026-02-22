#include "unittest/storage.h"

#include <cerrno>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

#include "esp_err.h"
#include "esp_spiffs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../serial/log.h"

namespace UnitTest
{
    namespace
    {
        constexpr const char *kTag = "STORTEST";
        constexpr const char *kMountCandidates[] = {"/spiffs", "/storage", "/config", "/littlefs", "/data", "/fatfs"};

        constexpr std::size_t kInitialSize = 256u * 1024u;
        constexpr std::size_t kOverwriteOffset = 64u * 1024u;
        constexpr std::size_t kOverwriteSize = 64u * 1024u;
        constexpr std::size_t kAppendSize = 128u * 1024u;
        constexpr std::size_t kChunkSize = 1024u;

        inline std::uint8_t patternA(std::size_t offset)
        {
            return static_cast<std::uint8_t>((offset * 17u + 0x33u) & 0xFFu);
        }

        inline std::uint8_t patternB(std::size_t offset)
        {
            return static_cast<std::uint8_t>((offset * 29u + 0xA5u) & 0xFFu);
        }

        inline std::uint8_t patternC(std::size_t offset)
        {
            return static_cast<std::uint8_t>((offset * 41u + 0x5Au) & 0xFFu);
        }

        bool hasDirectory(const char *path)
        {
            struct stat st{};
            if (stat(path, &st) != 0)
            {
                return false;
            }
            return (st.st_mode & S_IFDIR) != 0;
        }

        const char *detectMountPath()
        {
            for (const char *candidate : kMountCandidates)
            {
                if (hasDirectory(candidate))
                {
                    return candidate;
                }
            }
            return nullptr;
        }

        bool tryMountStoragePartition(const char *basePath, const char *partitionLabel, const char *&mountedLabel)
        {
            esp_vfs_spiffs_conf_t conf = {};
            conf.base_path = basePath;
            conf.partition_label = partitionLabel;
            conf.max_files = 4;
            conf.format_if_mount_failed = false;

            esp_err_t err = esp_vfs_spiffs_register(&conf);
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
            // Try mounting with specific storage partition first
            if (tryMountStoragePartition(basePath, "storage", mountedLabel))
            {
                return true;
            }

            // Try mounting with unspecified partition (any SPIFFS partition)
            esp_vfs_spiffs_conf_t conf = {};
            conf.base_path = basePath;
            conf.partition_label = nullptr;
            conf.max_files = 4;
            conf.format_if_mount_failed = true;

            esp_err_t err = esp_vfs_spiffs_register(&conf);
            if (err == ESP_OK)
            {
                mountedLabel = nullptr;
                Log::sys_info(kTag, "Mounted generic SPIFFS partition at " + String(basePath));
                return true;
            }

            Log::sys_error(kTag, "SPIFFS mount failed: " + String(esp_err_to_name(err)) + " (" + String(static_cast<int>(err)) + ")");
            return false;
        }

        void cleanupMount(const char *basePath, const char *mountedLabel)
        {
            if (!mountedLabel)
            {
                esp_vfs_spiffs_unregister(basePath);
            }
            else
            {
                esp_vfs_spiffs_unregister(mountedLabel);
            }
        }

        bool writeRange(FILE *file, std::size_t fileOffset, std::size_t length, std::uint8_t (*pat)(std::size_t))
        {
            if (fseek(file, static_cast<long>(fileOffset), SEEK_SET) != 0)
            {
                Log::sys_error(kTag, "fseek(write) failed: " + String(errno));
                return false;
            }

            static std::uint8_t buffer[kChunkSize];
            std::size_t written = 0;
            while (written < length)
            {
                const std::size_t n = (length - written > kChunkSize) ? kChunkSize : (length - written);
                for (std::size_t i = 0; i < n; ++i)
                {
                    buffer[i] = pat(fileOffset + written + i);
                }

                if (fwrite(buffer, 1u, n, file) != n)
                {
                    Log::sys_error(kTag, "fwrite failed after " + String(written) + " bytes");
                    return false;
                }
                written += n;

                if (((written / kChunkSize) & 0x1Fu) == 0u)
                {
                    vTaskDelay(1);
                }
            }

            if (fflush(file) != 0)
            {
                Log::sys_error(kTag, "fflush failed: " + String(errno));
                return false;
            }

            if (fsync(fileno(file)) != 0)
            {
                Log::sys_error(kTag, "fsync failed: " + String(errno));
                return false;
            }

            return true;
        }

        bool verifyPhaseFile(FILE *file, std::size_t totalSize)
        {
            if (fseek(file, 0, SEEK_SET) != 0)
            {
                Log::sys_error(kTag, "fseek(read) failed: " + String(errno));
                return false;
            }

            static std::uint8_t buffer[kChunkSize];
            std::size_t readOffset = 0;
            while (readOffset < totalSize)
            {
                const std::size_t n = (totalSize - readOffset > kChunkSize) ? kChunkSize : (totalSize - readOffset);
                if (fread(buffer, 1u, n, file) != n)
                {
                    Log::sys_error(kTag, "fread failed at offset " + String(readOffset));
                    return false;
                }

                for (std::size_t i = 0; i < n; ++i)
                {
                    const std::size_t pos = readOffset + i;
                    std::uint8_t expected;

                    if (pos < kInitialSize)
                    {
                        if (pos >= kOverwriteOffset && pos < (kOverwriteOffset + kOverwriteSize))
                        {
                            expected = patternB(pos);
                        }
                        else
                        {
                            expected = patternA(pos);
                        }
                    }
                    else
                    {
                        expected = patternC(pos - kInitialSize);
                    }

                    if (buffer[i] != expected)
                    {
                        Log::sys_error(kTag, "Data mismatch at offset " + String(pos));
                        return false;
                    }
                }

                readOffset += n;
                if (((readOffset / kChunkSize) & 0x1Fu) == 0u)
                {
                    vTaskDelay(1);
                }
            }

            return true;
        }

        bool existsFile(const String &path)
        {
            struct stat st{};
            return stat(path.c_str(), &st) == 0;
        }
    } // namespace

    bool runStorageTest()
    {
        bool mountedByTest = false;
        const char *spiffsLabel = nullptr;
        const char *mount = detectMountPath();

        if (!mount)
        {
            Log::sys_info(kTag, "No mounted storage path found. Attempting to mount storage partition...");
            if (!tryMountSpiffs("/storage", spiffsLabel))
            {
                Log::sys_error(kTag, "Could not mount storage partition");
                return false;
            }

            mount = "/storage";
            mountedByTest = true;
            Log::sys_info(kTag, "Mounted storage partition for test at /storage");
        }
        else
        {
            Log::sys_info(kTag, "Using existing storage mount at " + String(mount));
        }

        const String basePath = String(mount);
        const String testPath = basePath + "/hkv_storage_test.bin";
        const String renamedPath = basePath + "/hkv_storage_test_done.bin";

        remove(testPath.c_str());
        remove(renamedPath.c_str());

        FILE *file = fopen(testPath.c_str(), "wb+");
        if (!file)
        {
            Log::sys_error(kTag, "fopen create failed: " + String(errno));
            if (mountedByTest)
            {
                cleanupMount(basePath.c_str(), spiffsLabel);
            }
            return false;
        }

        if (!writeRange(file, 0u, kInitialSize, patternA))
        {
            fclose(file);
            remove(testPath.c_str());
            if (mountedByTest)
            {
                cleanupMount(basePath.c_str(), spiffsLabel);
            }
            return false;
        }

        if (!writeRange(file, kOverwriteOffset, kOverwriteSize, patternB))
        {
            fclose(file);
            remove(testPath.c_str());
            if (mountedByTest)
            {
                cleanupMount(basePath.c_str(), spiffsLabel);
            }
            return false;
        }

        if (!writeRange(file, kInitialSize, kAppendSize, patternC))
        {
            fclose(file);
            remove(testPath.c_str());
            if (mountedByTest)
            {
                cleanupMount(basePath.c_str(), spiffsLabel);
            }
            return false;
        }

        fclose(file);

        file = fopen(testPath.c_str(), "rb");
        if (!file)
        {
            Log::sys_error(kTag, "fopen read failed: " + String(errno));
            remove(testPath.c_str());
            if (mountedByTest)
            {
                cleanupMount(basePath.c_str(), spiffsLabel);
            }
            return false;
        }

        const std::size_t totalSize = kInitialSize + kAppendSize;
        const bool verified = verifyPhaseFile(file, totalSize);
        fclose(file);
        if (!verified)
        {
            remove(testPath.c_str());
            if (mountedByTest)
            {
                cleanupMount(basePath.c_str(), spiffsLabel);
            }
            return false;
        }

        if (rename(testPath.c_str(), renamedPath.c_str()) != 0)
        {
            Log::sys_error(kTag, "rename failed: " + String(errno));
            remove(testPath.c_str());
            if (mountedByTest)
            {
                cleanupMount(basePath.c_str(), spiffsLabel);
            }
            return false;
        }

        if (existsFile(testPath) || !existsFile(renamedPath))
        {
            Log::sys_error(kTag, "rename verification failed");
            remove(testPath.c_str());
            remove(renamedPath.c_str());
            if (mountedByTest)
            {
                cleanupMount(basePath.c_str(), spiffsLabel);
            }
            return false;
        }

        if (remove(renamedPath.c_str()) != 0)
        {
            Log::sys_error(kTag, "remove failed: " + String(errno));
            if (mountedByTest)
            {
                cleanupMount(basePath.c_str(), spiffsLabel);
            }
            return false;
        }

        if (existsFile(renamedPath))
        {
            Log::sys_error(kTag, "delete verification failed");
            if (mountedByTest)
            {
                cleanupMount(basePath.c_str(), spiffsLabel);
            }
            return false;
        }

        if (mountedByTest)
        {
            if (spiffsLabel)
            {
                esp_vfs_spiffs_unregister(spiffsLabel);
            }
            else
            {
                esp_vfs_spiffs_unregister(basePath.c_str());
            }
            Log::sys_info(kTag, "Storage partition unmounted");
        }

        Log::sys_info(kTag, "Storage integrity test successful on " + basePath + ", tested: " + String(totalSize) + " bytes");
        return true;
    }
} // namespace UnitTest
