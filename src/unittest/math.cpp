#include "unittest/math.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../serial/log.h"

namespace UnitTest
{
    namespace
    {
        constexpr const char *kTag = "MATHTEST";
        constexpr std::size_t kIterations = 200000u;

        inline bool almostEqual(double a, double b, double eps)
        {
            const double diff = std::fabs(a - b);
            return diff <= eps;
        }
    } // namespace

    bool runMathTest()
    {
        double acc = 0.0;
        double trigErrorMax = 0.0;

        for (std::size_t i = 1; i <= kIterations; ++i)
        {
            const double x = static_cast<double>(i) * 0.0005;
            const double y = static_cast<double>(i % 997u) * 0.001 + 1.0;

            const double s = std::sin(x);
            const double c = std::cos(x);
            const double trig = s * s + c * c;
            const double trigErr = std::fabs(trig - 1.0);
            if (trigErr > trigErrorMax)
            {
                trigErrorMax = trigErr;
            }

            if (!almostEqual(trig, 1.0, 1e-9))
            {
                Log::sys_error(kTag, "Trig identity instability at i=" + String(i) + ", err=" + String(trigErr, 12));
                return false;
            }

            const double l = std::log(y);
            const double e = std::exp(l);
            if (!almostEqual(e, y, 1e-10 * y))
            {
                Log::sys_error(kTag, "exp(log(x)) mismatch at i=" + String(i));
                return false;
            }

            const double r = std::sqrt(y * y);
            if (!almostEqual(r, y, 1e-10 * y))
            {
                Log::sys_error(kTag, "sqrt(x*x) mismatch at i=" + String(i));
                return false;
            }

            acc += (s + c + l + e / y + r / y);
            if ((i % 2000u) == 0u)
            {
                vTaskDelay(1);
            }
        }

        if (!std::isfinite(acc))
        {
            Log::sys_error(kTag, "Accumulator became non-finite");
            return false;
        }

        Log::sys_info(kTag, "Math stability test successful, iters=" + String(kIterations) + ", max-trig-err=" + String(trigErrorMax, 12) + ", checksum=" + String(acc, 6));
        return true;
    }
} // namespace UnitTest
