#include "./prove_priority_inheritance_correctness.h"

#include "../deadlock_starvation/priority_inversion.h"

namespace UnitTest
{
    bool runRtosFormalPriorityInheritanceCorrectnessTest()
    {
        return runRtosPriorityInversionTest();
    }
} // namespace UnitTest
