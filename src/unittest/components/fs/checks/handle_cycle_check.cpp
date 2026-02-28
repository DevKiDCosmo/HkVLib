#include "./handle_cycle_check.h"

#include <cerrno>
#include <cstdio>

#include "../fs_common.h"
#include "serial/log.h"

namespace UnitTest
{
    bool runFsHandleCycleCheck()
    {
        constexpr const char *kTag = "FSTEST";
        constexpr std::size_t kCycles = 128u;

        FsInternal::FsContext ctx;
        if (!FsInternal::ensureFsContext(ctx))
        {
            return false;
        }

        const String basePath = String(ctx.mount);
        const String path = basePath + "/hkv_fs_handle_cycle.bin";

        FsInternal::cleanupPathIfExists(path);

        for (std::size_t i = 0; i < kCycles; ++i)
        {
            FILE *file = fopen(path.c_str(), "ab");
            if (!file)
            {
                Log::sys_error(kTag, "cycle fopen failed at i=" + String(i) + ", errno=" + String(errno));
                FsInternal::cleanupPathIfExists(path);
                FsInternal::releaseFsContext(ctx);
                return false;
            }

            const unsigned char value = static_cast<unsigned char>(i & 0xFFu);
            const bool writeOk = fwrite(&value, 1u, 1u, file) == 1u;
            fclose(file);
            if (!writeOk)
            {
                Log::sys_error(kTag, "cycle fwrite failed at i=" + String(i));
                FsInternal::cleanupPathIfExists(path);
                FsInternal::releaseFsContext(ctx);
                return false;
            }
        }

        FILE *verify = fopen(path.c_str(), "rb");
        if (!verify)
        {
            Log::sys_error(kTag, "cycle verify fopen failed: " + String(errno));
            FsInternal::cleanupPathIfExists(path);
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        if (fseek(verify, 0, SEEK_END) != 0)
        {
            fclose(verify);
            Log::sys_error(kTag, "cycle verify fseek failed: " + String(errno));
            FsInternal::cleanupPathIfExists(path);
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        const long size = ftell(verify);
        fclose(verify);
        if (size != static_cast<long>(kCycles))
        {
            Log::sys_error(kTag, "cycle verify size mismatch: got=" + String(size) + ", expected=" + String(static_cast<long>(kCycles)));
            FsInternal::cleanupPathIfExists(path);
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        FsInternal::cleanupPathIfExists(path);
        Log::sys_info(kTag, "Handle-cycle check successful, cycles=" + String(kCycles));
        FsInternal::releaseFsContext(ctx);
        return true;
    }
} // namespace UnitTest
