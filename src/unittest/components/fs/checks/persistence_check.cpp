#include "./persistence_check.h"

#include <cerrno>
#include <cstdio>
#include <cstring>

#include "../fs_common.h"
#include "serial/log.h"

namespace UnitTest
{
    bool runFsPersistenceCheck()
    {
        constexpr const char *kTag = "FSTEST";

        FsInternal::FsContext ctx;
        if (!FsInternal::ensureFsContext(ctx))
        {
            return false;
        }

        const String basePath = String(ctx.mount);
        const String path = basePath + "/hkv_fs_persist.txt";
        static const char payload[] = "persistent-data-v1";

        FsInternal::cleanupPathIfExists(path);

        FILE *file = fopen(path.c_str(), "wb");
        if (!file)
        {
            Log::sys_error(kTag, "persistence create failed: " + String(errno));
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        const bool writeOk = fwrite(payload, 1u, sizeof(payload), file) == sizeof(payload);
        fclose(file);
        if (!writeOk)
        {
            Log::sys_error(kTag, "persistence write failed");
            FsInternal::cleanupPathIfExists(path);
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        char buffer[sizeof(payload)] = {};
        file = fopen(path.c_str(), "rb");
        if (!file)
        {
            Log::sys_error(kTag, "persistence reopen failed: " + String(errno));
            FsInternal::cleanupPathIfExists(path);
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        const bool readOk = fread(buffer, 1u, sizeof(buffer), file) == sizeof(buffer);
        fclose(file);
        if (!readOk || strcmp(buffer, payload) != 0)
        {
            Log::sys_error(kTag, "persistence verification mismatch");
            FsInternal::cleanupPathIfExists(path);
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        FsInternal::cleanupPathIfExists(path);
        Log::sys_info(kTag, "Persistence check successful");
        FsInternal::releaseFsContext(ctx);
        return true;
    }
} // namespace UnitTest
