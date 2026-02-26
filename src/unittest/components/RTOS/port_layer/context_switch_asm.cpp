#include "./context_switch_asm.h"

#include "../scheduler/context_switch_integrity.h"

namespace UnitTest
{
    bool runRtosPortContextSwitchAsmCorrectnessTest()
    {
        return runRtosContextSwitchIntegrityTest();
    }
} // namespace UnitTest
