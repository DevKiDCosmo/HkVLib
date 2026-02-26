#include "./do_178c.h"

#include "../requirements/mcdc_coverage.h"
#include "../requirements/requirement_to_test_mapping.h"
#include "../requirements/structural_coverage.h"
#include "../requirements/traceability.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runDo178cCertificationTest()
    {
        constexpr const char *kTag = "CERT_DO178C";

        bool ok = true;
        ok = runCertificationTraceabilityTest() && ok;
        ok = runCertificationRequirementToTestMappingTest() && ok;
        ok = runCertificationMcdcCoverageTest() && ok;
        ok = runCertificationStructuralCoverageTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "DO-178C baseline failed");
            return false;
        }

        Log::sys_info(kTag, "DO-178C baseline successful");
        return true;
    }
} // namespace UnitTest
