#include "./fs.h"
#include "./checks/directory_check.h"
#include "./checks/error_check.h"
#include "./checks/file_check.h"
#include "./checks/handle_cycle_check.h"
#include "./checks/mount_check.h"
#include "./checks/persistence_check.h"

#include "serial/log.h"

namespace UnitTest
{
    namespace
    {
        constexpr const char *kTag = "FSTEST";
    }

    bool runFsMainCheck()
    {
        Log::sys_info(kTag, "Starting filesystem checks");

        bool ok = true;
        ok = runFsMountCheck() && ok;
        ok = runFsFileCheck() && ok;
        ok = runFsDirectoryCheck() && ok;
        ok = runFsErrorCheck() && ok;
        ok = runFsPersistenceCheck() && ok;
        ok = runFsHandleCycleCheck() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more filesystem checks failed");
            return false;
        }

        Log::sys_info(kTag, "Filesystem checks successful");
        return true;
    }
} // namespace UnitTest
