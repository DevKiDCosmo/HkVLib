#include "./error_check.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <dirent.h>

#include "../fs_common.h"
#include "serial/log.h"

namespace UnitTest
{
    bool runFsErrorCheck()
    {
        constexpr const char *kTag = "FSTEST";

        FsInternal::FsContext ctx;
        if (!FsInternal::ensureFsContext(ctx))
        {
            return false;
        }

        const String basePath = String(ctx.mount);
        const String missingFile = basePath + "/hkv_missing_file.bin";
        const String missingDir = basePath + "/hkv_missing_dir";

        errno = 0;
        FILE *missing = fopen(missingFile.c_str(), "rb");
        const int openErr = errno;
        if (missing)
        {
            fclose(missing);
            Log::sys_error(kTag, "Opening missing file unexpectedly succeeded");
            FsInternal::releaseFsContext(ctx);
            return false;
        }
        if (openErr == 0)
        {
            Log::sys_error(kTag, "Opening missing file failed without errno");
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        errno = 0;
        const int removeResult = remove(missingFile.c_str());
        const int removeErr = errno;
        if (removeResult == 0)
        {
            Log::sys_error(kTag, "Removing missing file unexpectedly succeeded");
            FsInternal::releaseFsContext(ctx);
            return false;
        }
        if (removeErr == 0)
        {
            Log::sys_error(kTag, "Removing missing file failed without errno");
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        FsInternal::cleanupPathIfExists(missingDir);

        errno = 0;
        DIR *dir = opendir(missingDir.c_str());
        int openDirErr = errno;
        if (dir)
        {
            bool foundEntries = false;
            while (dirent *entry = readdir(dir))
            {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                {
                    continue;
                }

                foundEntries = true;
                break;
            }
            closedir(dir);

            if (foundEntries)
            {
                Log::sys_error(kTag, "Missing directory unexpectedly resolved to existing entries");
                FsInternal::releaseFsContext(ctx);
                return false;
            }

            openDirErr = 0;
            Log::sys_warning(kTag, "opendir on missing directory succeeded due FS semantics, treated as acceptable");
        }
        else if (openDirErr == 0)
        {
            Log::sys_error(kTag, "Opening missing directory failed without errno");
            FsInternal::releaseFsContext(ctx);
            return false;
        }

        Log::sys_info(kTag, "Error-path check successful (open errno=" + String(openErr) + ", remove errno=" + String(removeErr) + ", opendir errno=" + String(openDirErr) + ")");
        FsInternal::releaseFsContext(ctx);
        return true;
    }
} // namespace UnitTest
