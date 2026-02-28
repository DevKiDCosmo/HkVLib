#pragma once

namespace UnitTest
{
    bool runFsMountCheck();
    bool runFsFileCheck();
    bool runFsDirectoryCheck();
    bool runFsErrorCheck();
    bool runFsPersistenceCheck();
    bool runFsHandleCycleCheck();
    bool runFsMainCheck();
}
