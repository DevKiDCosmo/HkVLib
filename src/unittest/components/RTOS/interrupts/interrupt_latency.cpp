#include "./interrupt_latency.h"

#include <cstdint>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosInterruptLatencyTest()
    {
        constexpr const char *kTag = "RTOS_INT_LAT";
        constexpr int kWarmupSamples = 10;
        constexpr int kMeasureSamples = 20;
        constexpr std::int64_t kAbsoluteMinThresholdUs = 3000;
        constexpr std::int64_t kBaselineMarginUs = 1000;
        static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

        std::int64_t warmupMaxUs = 0;
        std::int64_t warmupSumUs = 0;

        for (int sample = 0; sample < kWarmupSamples; ++sample)
        {
            const std::int64_t start = esp_timer_get_time();
            taskENTER_CRITICAL(&mux);
            for (volatile std::uint32_t spin = 0u; spin < 300u; ++spin)
            {
            }
            taskEXIT_CRITICAL(&mux);
            const std::int64_t elapsed = esp_timer_get_time() - start;

            warmupSumUs += elapsed;
            if (elapsed > warmupMaxUs)
            {
                warmupMaxUs = elapsed;
            }

            vTaskDelay(1);
        }

        const std::int64_t warmupAvgUs = (kWarmupSamples > 0) ? (warmupSumUs / kWarmupSamples) : 0;
        std::int64_t adaptiveThresholdUs = warmupMaxUs + warmupAvgUs + kBaselineMarginUs;
        if (adaptiveThresholdUs < kAbsoluteMinThresholdUs)
        {
            adaptiveThresholdUs = kAbsoluteMinThresholdUs;
        }

        std::int64_t measuredMaxUs = 0;
        for (int sample = 0; sample < kMeasureSamples; ++sample)
        {
            const std::int64_t start = esp_timer_get_time();
            taskENTER_CRITICAL(&mux);
            for (volatile std::uint32_t spin = 0u; spin < 300u; ++spin)
            {
            }
            taskEXIT_CRITICAL(&mux);
            const std::int64_t elapsed = esp_timer_get_time() - start;

            if (elapsed > measuredMaxUs)
            {
                measuredMaxUs = elapsed;
            }

            vTaskDelay(1);
        }

        if (measuredMaxUs > adaptiveThresholdUs)
        {
            Log::sys_error(
                kTag,
                "Interrupt latency proxy too high: measured-max=" + String(static_cast<long long>(measuredMaxUs)) +
                    " us, threshold=" + String(static_cast<long long>(adaptiveThresholdUs)) +
                    " us, warmup-max=" + String(static_cast<long long>(warmupMaxUs)) +
                    " us, warmup-avg=" + String(static_cast<long long>(warmupAvgUs)) + " us");
            return false;
        }

        Log::sys_info(
            kTag,
            "Interrupt latency baseline successful: measured-max=" + String(static_cast<long long>(measuredMaxUs)) +
                " us, threshold=" + String(static_cast<long long>(adaptiveThresholdUs)) +
                " us, warmup-max=" + String(static_cast<long long>(warmupMaxUs)) +
                " us, warmup-avg=" + String(static_cast<long long>(warmupAvgUs)) + " us");
        return true;
    }
} // namespace UnitTest
