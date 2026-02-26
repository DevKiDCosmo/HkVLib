#include "./path_coverage.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        int pathFunction(const bool a, const bool b)
        {
            if (a && b)
            {
                return 3;
            }

            if (a)
            {
                return 2;
            }

            if (b)
            {
                return 1;
            }

            return 0;
        }
    } // namespace

    bool runRtosPathCoverageTest()
    {
        constexpr const char *kTag = "RTOS_COV_PATH";

        if (pathFunction(false, false) != 0 ||
            pathFunction(false, true) != 1 ||
            pathFunction(true, false) != 2 ||
            pathFunction(true, true) != 3)
        {
            Log::sys_error(kTag, "Path coverage combinations failed");
            return false;
        }

        Log::sys_info(kTag, "Path coverage baseline successful");
        return true;
    }
} // namespace UnitTest
