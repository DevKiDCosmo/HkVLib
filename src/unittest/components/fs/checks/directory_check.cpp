#include "./directory_check.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../fs_common.h"
#include "serial/log.h"

namespace UnitTest
{
    namespace
    {
        constexpr int kErrNotSupported = 134;

        bool isDirectoryUnsupportedErrno(int code)
        {
            return code == ENOSYS || code == ENOTSUP || code == EOPNOTSUPP || code == kErrNotSupported;
        }
    }

    bool runFsDirectoryCheck()
    {
        constexpr const char *kTag = "FSTEST";

        FsInternal::FsContext ctx;
        if (!FsInternal::ensureFsContext(ctx))
        {
            return false;
        }

        const String basePath = String(ctx.mount);
        const String dirPath = basePath + "/hkv_fs_dir";
        const String nestedPath = dirPath + "/nested";
        const String filePath = nestedPath + "/probe.txt";

        FsInternal::cleanupPathIfExists(filePath);
        FsInternal::cleanupPathIfExists(nestedPath);
        FsInternal::cleanupPathIfExists(dirPath);

        if (mkdir(dirPath.c_str(), 0777) != 0)
        {
            const int mkErr = errno;
            if (isDirectoryUnsupportedErrno(mkErr))
            {
                Log::sys_warning(kTag, "Directory operations are not supported by this FS, skipping directory check");
                FsInternal::releaseFsContext(ctx);
                return true;
            }

            Log::sys_error(kTag, "mkdir dir failed: " + String(errno));
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        if (mkdir(nestedPath.c_str(), 0777) != 0)
        {
            Log::sys_error(kTag, "mkdir nested failed: " + String(errno));
            FsInternal::cleanupPathIfExists(dirPath);
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        FILE *file = fopen(filePath.c_str(), "wb");
        if (!file)
        {
            Log::sys_error(kTag, "fopen probe file failed: " + String(errno));
            FsInternal::cleanupPathIfExists(nestedPath);
            FsInternal::cleanupPathIfExists(dirPath);
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        static const char payload[] = "hkv-fs";
        const bool writeOk = fwrite(payload, 1u, sizeof(payload), file) == sizeof(payload);
        fclose(file);
        if (!writeOk)
        {
            Log::sys_error(kTag, "write probe file failed");
            FsInternal::cleanupPathIfExists(filePath);
            FsInternal::cleanupPathIfExists(nestedPath);
            FsInternal::cleanupPathIfExists(dirPath);
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        if (rmdir(nestedPath.c_str()) == 0)
        {
            Log::sys_error(kTag, "rmdir on non-empty directory unexpectedly succeeded");
            FsInternal::cleanupPathIfExists(filePath);
            FsInternal::cleanupPathIfExists(nestedPath);
            FsInternal::cleanupPathIfExists(dirPath);
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        bool listedProbe = false;
        DIR *dir = opendir(nestedPath.c_str());
        if (!dir)
        {
            Log::sys_error(kTag, "opendir failed: " + String(errno));
            FsInternal::cleanupPathIfExists(filePath);
            FsInternal::cleanupPathIfExists(nestedPath);
            FsInternal::cleanupPathIfExists(dirPath);
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        while (dirent *entry = readdir(dir))
        {
            if (strcmp(entry->d_name, "probe.txt") == 0)
            {
                listedProbe = true;
                break;
            }
        }
        closedir(dir);

        if (!listedProbe)
        {
            Log::sys_error(kTag, "Directory listing does not contain probe.txt");
            FsInternal::cleanupPathIfExists(filePath);
            FsInternal::cleanupPathIfExists(nestedPath);
            FsInternal::cleanupPathIfExists(dirPath);
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        FsInternal::cleanupPathIfExists(filePath);
        const bool removeNestedOk = rmdir(nestedPath.c_str()) == 0;
        const bool removeDirOk = rmdir(dirPath.c_str()) == 0;
        if (!removeNestedOk || !removeDirOk)
        {
            Log::sys_error(kTag, "rmdir failed (nested=" + String(removeNestedOk ? "ok" : "fail") + ", root=" + String(removeDirOk ? "ok" : "fail") + ")");
            FsInternal::cleanupPathIfExists(nestedPath);
            FsInternal::cleanupPathIfExists(dirPath);
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        Log::sys_info(kTag, "Directory check successful");
        FsInternal::releaseFsContext(ctx);
        return true;
    }
} // namespace UnitTest
