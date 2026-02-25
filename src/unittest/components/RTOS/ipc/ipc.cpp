#include "./ipc.h"

#include "./mailbox.h"
#include "./queue.h"
#include "./shared_memory.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosIpcSuite()
    {
        constexpr const char *kTag = "RTOS_IPC";

        bool ok = true;
        ok = runRtosQueueTest() && ok;
        ok = runRtosMailboxTest() && ok;
        ok = runRtosSharedMemoryTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more IPC tests failed");
            return false;
        }

        Log::sys_info(kTag, "IPC tests successful");
        return true;
    }
} // namespace UnitTest
