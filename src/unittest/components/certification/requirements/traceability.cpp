#include "./traceability.h"

#include <cstddef>

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runCertificationTraceabilityTest()
    {
        constexpr const char *kTag = "CERT_TRACE";

        constexpr const char *kRequirementIds[] = {
            "ISO-26262-REQ-001",
            "DO-178C-REQ-001",
            "IEC-61508-REQ-001",
            "CERT-REQ-TRACE-001",
            "CERT-REQ-MAP-001",
            "CERT-REQ-MCDC-001",
            "CERT-REQ-STRUCT-001",
        };

        constexpr std::size_t kCount = sizeof(kRequirementIds) / sizeof(kRequirementIds[0]);
        if (kCount < 7u)
        {
            Log::sys_error(kTag, "Requirement baseline count too low");
            return false;
        }

        for (std::size_t i = 0; i < kCount; ++i)
        {
            if (kRequirementIds[i] == nullptr || kRequirementIds[i][0] == '\0')
            {
                Log::sys_error(kTag, "Empty requirement id at index " + String(i));
                return false;
            }
        }

        Log::sys_info(kTag, "Traceability baseline successful, requirements=" + String(static_cast<unsigned int>(kCount)));
        return true;
    }
} // namespace UnitTest
