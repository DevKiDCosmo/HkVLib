#include "./requirement_to_test_mapping.h"

#include <cstddef>

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        struct RequirementMapping
        {
            const char *requirement;
            const char *testSymbol;
        };
    } // namespace

    bool runCertificationRequirementToTestMappingTest()
    {
        constexpr const char *kTag = "CERT_MAP";

        constexpr RequirementMapping kMappings[] = {
            {"ISO-26262-REQ-001", "runIso26262CertificationTest"},
            {"DO-178C-REQ-001", "runDo178cCertificationTest"},
            {"IEC-61508-REQ-001", "runIec61508CertificationTest"},
            {"CERT-REQ-TRACE-001", "runCertificationTraceabilityTest"},
            {"CERT-REQ-MAP-001", "runCertificationRequirementToTestMappingTest"},
            {"CERT-REQ-MCDC-001", "runCertificationMcdcCoverageTest"},
            {"CERT-REQ-STRUCT-001", "runCertificationStructuralCoverageTest"},
        };

        constexpr std::size_t kCount = sizeof(kMappings) / sizeof(kMappings[0]);
        for (std::size_t i = 0; i < kCount; ++i)
        {
            if (kMappings[i].requirement == nullptr || kMappings[i].requirement[0] == '\0' ||
                kMappings[i].testSymbol == nullptr || kMappings[i].testSymbol[0] == '\0')
            {
                Log::sys_error(kTag, "Invalid mapping at index " + String(i));
                return false;
            }
        }

        for (std::size_t i = 0; i < kCount; ++i)
        {
            for (std::size_t j = i + 1u; j < kCount; ++j)
            {
                if (String(kMappings[i].requirement) == String(kMappings[j].requirement))
                {
                    Log::sys_error(kTag, "Duplicate requirement mapping for " + String(kMappings[i].requirement));
                    return false;
                }
            }
        }

        Log::sys_info(kTag, "Requirement-to-test mapping baseline successful");
        return true;
    }
} // namespace UnitTest
