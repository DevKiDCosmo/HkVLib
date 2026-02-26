#include "./structural_coverage.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        enum class State
        {
            Idle,
            Running,
            Fault,
        };

        State transition(const State current, const bool startSignal, const bool faultSignal)
        {
            if (faultSignal)
            {
                return State::Fault;
            }

            if (current == State::Idle && startSignal)
            {
                return State::Running;
            }

            if (current == State::Running && !startSignal)
            {
                return State::Idle;
            }

            return current;
        }
    } // namespace

    bool runCertificationStructuralCoverageTest()
    {
        constexpr const char *kTag = "CERT_STRUCT";

        const State s1 = transition(State::Idle, true, false);
        const State s2 = transition(s1, false, false);
        const State s3 = transition(s2, false, true);

        if (s1 != State::Running || s2 != State::Idle || s3 != State::Fault)
        {
            Log::sys_error(kTag, "Structural state transitions failed");
            return false;
        }

        Log::sys_info(kTag, "Structural coverage baseline successful");
        return true;
    }
} // namespace UnitTest
