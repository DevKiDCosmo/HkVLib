#include "./mount_check.h"

#include <cerrno>
#include <cstdio>

#include "../fs_common.h"
#include "serial/log.h"

namespace UnitTest
{
    bool runFsMountCheck()
    {
        constexpr const char *kTag = "FSTEST";

        FsInternal::FsContext ctx;
        if (!FsInternal::ensureFsContext(ctx))
        {
            return false;
        }

        const String probePath = String(ctx.mount) + "/hkv_fs_mount_probe.tmp";

        FILE *probe = fopen(probePath.c_str(), "wb");
        if (!probe)
        {
            Log::sys_error(kTag, "Mount probe open failed at " + String(ctx.mount) + ", errno=" + String(errno));
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        static const char payload[] = "m";
        const bool writeOk = fwrite(payload, 1u, sizeof(payload), probe) == sizeof(payload);
        fclose(probe);

        const bool removeOk = (remove(probePath.c_str()) == 0) || !FsInternal::existsPath(probePath);
        const bool ok = writeOk && removeOk;

        if (!ok)
        {
            Log::sys_error(kTag, "Mount probe write/remove failed at " + String(ctx.mount));
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        Log::sys_info(kTag, "Mount check successful at " + String(ctx.mount));

        FsInternal::releaseFsContext(ctx);
        return ok;
    }
} // namespace UnitTest
