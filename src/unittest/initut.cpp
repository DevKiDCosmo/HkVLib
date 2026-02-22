#include "initut.h"
#include "../utility/init.h"
#include "../serial/log.h"

bool UnitTest::initUnitTests()
{
    if (!Init::value())
    {
        Log::sys_info("INITTEST", "Initialization test successful, Init::value() returned false as expected");
        return true;
    }

    // TODO: Check more value after init.

    Log::sys_error("INITTEST", "Initialization test failed, Init::value() did not return false");
    Log::sys_error("INITTEST", "This likely indicates that the Init class was not properly initialized or reset before this test. Tampered initialization state can cause this test to fail. Please ensure that the Init class is correctly implemented and that its state is reset before running this test.");
    return false;
}

// TODO: Init Daemon Watchdog. Checks value while init.