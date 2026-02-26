#include "./iso_26262.h"

#include "../../RTOS/safety_critical/safety_critical.h"
#include "../requirements/requirement_to_test_mapping.h"
#include "../requirements/traceability.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runIso26262CertificationTest()
    {
        constexpr const char *kTag = "CERT_ISO26262";

        bool ok = true;
        ok = runRtosSafetyCriticalSuite() && ok;
        ok = runCertificationTraceabilityTest() && ok;
        ok = runCertificationRequirementToTestMappingTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "ISO 26262 baseline failed");
            return false;
        }

        Log::sys_info(kTag, "ISO 26262 baseline successful");
        return true;
    }
} // namespace UnitTest
