#include "./model_check_scheduling.h"

#include "../determinism/schedule_repeatability.h"

namespace UnitTest
{
    bool runRtosFormalModelCheckSchedulingTest()
    {
        return runRtosScheduleRepeatabilityTest();
    }
} // namespace UnitTest
