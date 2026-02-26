#include "./branch_coverage.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        int branchFunction(const int value)
        {
            if (value > 0)
            {
                return 1;
            }

            return -1;
        }
    } // namespace

    bool runRtosBranchCoverageTest()
    {
        constexpr const char *kTag = "RTOS_COV_BR";

        if (branchFunction(1) != 1)
        {
            Log::sys_error(kTag, "Positive branch not covered correctly");
            return false;
        }

        if (branchFunction(0) != -1)
        {
            Log::sys_error(kTag, "Negative branch not covered correctly");
            return false;
        }

        Log::sys_info(kTag, "Branch coverage baseline successful");
        return true;
    }
} // namespace UnitTest
