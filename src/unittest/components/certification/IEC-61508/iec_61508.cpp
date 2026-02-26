#include "./iec_61508.h"

#include "../../RTOS/formal_verification/formal_verification.h"
#include "../requirements/requirement_to_test_mapping.h"
#include "../requirements/structural_coverage.h"
#include "../requirements/traceability.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runIec61508CertificationTest()
    {
        constexpr const char *kTag = "CERT_IEC61508";

        bool ok = true;
        ok = runRtosFormalVerificationSuite() && ok;
        ok = runCertificationTraceabilityTest() && ok;
        ok = runCertificationRequirementToTestMappingTest() && ok;
        ok = runCertificationStructuralCoverageTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "IEC 61508 baseline failed");
            return false;
        }

        Log::sys_info(kTag, "IEC 61508 baseline successful");
        return true;
    }
} // namespace UnitTest
