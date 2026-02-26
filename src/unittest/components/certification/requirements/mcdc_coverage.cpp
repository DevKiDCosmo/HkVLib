#include "./mcdc_coverage.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        bool decision(const bool conditionA, const bool conditionB)
        {
            return conditionA && conditionB;
        }
    } // namespace

    bool runCertificationMcdcCoverageTest()
    {
        constexpr const char *kTag = "CERT_MCDC";

        const bool tt = decision(true, true);
        const bool tf = decision(true, false);
        const bool ft = decision(false, true);
        const bool ff = decision(false, false);

        if (!tt || tf || ft || ff)
        {
            Log::sys_error(kTag, "Truth table failed");
            return false;
        }

        const bool aIndependent = (decision(true, true) != decision(false, true));
        const bool bIndependent = (decision(true, true) != decision(true, false));
        if (!aIndependent || !bIndependent)
        {
            Log::sys_error(kTag, "MC/DC independence not satisfied");
            return false;
        }

        Log::sys_info(kTag, "MC/DC baseline successful");
        return true;
    }
} // namespace UnitTest
