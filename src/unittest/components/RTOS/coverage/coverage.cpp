#include "./coverage.h"

#include "./branch_coverage.h"
#include "./error_branch_coverage.h"
#include "./isr_coverage.h"
#include "./path_coverage.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosCoverageSuite()
    {
        constexpr const char *kTag = "RTOS_COV";

        bool ok = true;
        ok = runRtosBranchCoverageTest() && ok;
        ok = runRtosPathCoverageTest() && ok;
        ok = runRtosIsrCoverageTest() && ok;
        ok = runRtosErrorBranchCoverageTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more coverage tests failed");
            return false;
        }

        Log::sys_info(kTag, "Coverage tests successful");
        return true;
    }
} // namespace UnitTest
