#include "./random_interrupt_timing.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        std::uint32_t nextSeed(std::uint32_t seed)
        {
            return (seed * 1103515245u) + 12345u;
        }
    } // namespace

    bool runRtosRandomInterruptTimingFuzzTest()
    {
        constexpr const char *kTag = "RTOS_FUZZ_IRQ";
        static portMUX_TYPE criticalMux = portMUX_INITIALIZER_UNLOCKED;

        std::uint32_t seed = 0xAA551122u;
        TickType_t previous = xTaskGetTickCount();

        for (int i = 0; i < 24; ++i)
        {
            seed = nextSeed(seed);
            const TickType_t delayTicks = static_cast<TickType_t>(seed % 3u);

            if (delayTicks > 0u)
            {
                vTaskDelay(delayTicks);
            }

            taskENTER_CRITICAL(&criticalMux);
            taskEXIT_CRITICAL(&criticalMux);

            const TickType_t current = xTaskGetTickCount();
            if (current < previous)
            {
                Log::sys_error(kTag, "Tick regression detected");
                return false;
            }

            previous = current;
        }

        Log::sys_info(kTag, "Random interrupt timing successful");
        return true;
    }
} // namespace UnitTest
