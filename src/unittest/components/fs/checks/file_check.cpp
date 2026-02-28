#include "./file_check.h"

#include <cerrno>
#include <cstdio>
#include <cstdint>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../fs_common.h"
#include "serial/log.h"

namespace UnitTest
{
    namespace
    {
        constexpr const char *kTag = "FSTEST";
        constexpr std::size_t kInitialSize = 8u * 1024u;
        constexpr std::size_t kOverwriteOffset = 2u * 1024u;
        constexpr std::size_t kOverwriteSize = 2u * 1024u;
        constexpr std::size_t kAppendSize = 4u * 1024u;
        constexpr std::size_t kChunkSize = 1024u;
        constexpr int kErrNoSys = 88;
        bool g_fsyncUnsupported = false;

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

            if (g_fsyncUnsupported)
            {
                return true;
            }

            if (fsync(fileno(file)) != 0)
            {
                const int syncErr = errno;
                if (syncErr == ENOSYS || syncErr == kErrNoSys)
                {
                    g_fsyncUnsupported = true;
                    Log::sys_warning(kTag, "fsync unsupported on this FS, continuing with fflush only");
                    return true;
                }

                Log::sys_error(kTag, "fsync failed: " + String(syncErr));
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
    } // namespace

    bool runFsFileCheck()
    {
        FsInternal::FsContext ctx;
        if (!FsInternal::ensureFsContext(ctx))
        {
            return false;
        }

        const String basePath = String(ctx.mount);
        const String testPath = basePath + "/hkv_fs_test.bin";
        const String renamedPath = basePath + "/hkv_fs_test_done.bin";

        FsInternal::cleanupPathIfExists(testPath);
        FsInternal::cleanupPathIfExists(renamedPath);

        FILE *file = fopen(testPath.c_str(), "wb+");
        if (!file)
        {
            Log::sys_error(kTag, "fopen create failed: " + String(errno));
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        bool ok = true;
        ok = ok && writeRange(file, 0u, kInitialSize, patternA);
        ok = ok && writeRange(file, kOverwriteOffset, kOverwriteSize, patternB);
        ok = ok && writeRange(file, kInitialSize, kAppendSize, patternC);
        fclose(file);

        if (!ok)
        {
            FsInternal::cleanupPathIfExists(testPath);
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        file = fopen(testPath.c_str(), "rb");
        if (!file)
        {
            Log::sys_error(kTag, "fopen read failed: " + String(errno));
            FsInternal::cleanupPathIfExists(testPath);
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        const std::size_t totalSize = kInitialSize + kAppendSize;
        ok = verifyPhaseFile(file, totalSize);
        fclose(file);
        if (!ok)
        {
            FsInternal::cleanupPathIfExists(testPath);
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        if (rename(testPath.c_str(), renamedPath.c_str()) != 0)
        {
            Log::sys_error(kTag, "rename failed: " + String(errno));
            FsInternal::cleanupPathIfExists(testPath);
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        if (FsInternal::existsPath(testPath) || !FsInternal::existsPath(renamedPath))
        {
            Log::sys_error(kTag, "rename verification failed");
            FsInternal::cleanupPathIfExists(testPath);
            FsInternal::cleanupPathIfExists(renamedPath);
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        if (remove(renamedPath.c_str()) != 0)
        {
            Log::sys_error(kTag, "remove failed: " + String(errno));
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        const bool deleteOk = !FsInternal::existsPath(renamedPath);
        if (!deleteOk)
        {
            Log::sys_error(kTag, "delete verification failed");
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        Log::sys_info(kTag, "File check successful, tested=" + String(totalSize) + " bytes");
        FsInternal::releaseFsContext(ctx);
        return true;
    }
} // namespace UnitTest
