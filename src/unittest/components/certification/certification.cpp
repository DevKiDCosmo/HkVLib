#include "./certification.h"

#include "./DO-178C/do_178c.h"
#include "./IEC-61508/iec_61508.h"
#include "./ISO-26262/iso_26262.h"
#include "./requirements/mcdc_coverage.h"
#include "./requirements/requirement_to_test_mapping.h"
#include "./requirements/structural_coverage.h"
#include "./requirements/traceability.h"

#include "../../../serial/log.h"

namespace UnitTest
{
    bool runCertificationSuite()
    {
        constexpr const char *kTag = "CERT";

        bool ok = true;
        ok = runIso26262CertificationTest() && ok;
        ok = runDo178cCertificationTest() && ok;
        ok = runIec61508CertificationTest() && ok;

        ok = runCertificationTraceabilityTest() && ok;
        ok = runCertificationRequirementToTestMappingTest() && ok;
        ok = runCertificationMcdcCoverageTest() && ok;
        ok = runCertificationStructuralCoverageTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more certification tests failed");
            return false;
        }

        Log::sys_info(kTag, "Certification suite successful");
        return true;
    }
} // namespace UnitTest
