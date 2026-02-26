#include "./random_semaphore_interactions.h"

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    namespace
    {
        std::uint32_t nextSeed(std::uint32_t seed)
        {
            return (seed * 22695477u) + 1u;
        }
    } // namespace

    bool runRtosRandomSemaphoreInteractionsFuzzTest()
    {
        constexpr const char *kTag = "RTOS_FUZZ_SEM";

        SemaphoreHandle_t semaphore = xSemaphoreCreateBinary();
        if (semaphore == nullptr)
        {
            Log::sys_error(kTag, "Failed to create semaphore");
            return false;
        }

        std::uint32_t seed = 0x13572468u;
        bool tokenAvailable = false;

        for (int i = 0; i < 64; ++i)
        {
            seed = nextSeed(seed);
            const bool doGive = ((seed & 0x1u) == 0u);

            if (doGive)
            {
                xSemaphoreGive(semaphore);
                tokenAvailable = true;
            }
            else
            {
                const BaseType_t taken = xSemaphoreTake(semaphore, 0);
                if (tokenAvailable && taken != pdTRUE)
                {
                    Log::sys_error(kTag, "Expected token, but take failed at step " + String(i));
                    vSemaphoreDelete(semaphore);
                    return false;
                }

                if (!tokenAvailable && taken == pdTRUE)
                {
                    Log::sys_error(kTag, "Unexpected token, but take succeeded at step " + String(i));
                    vSemaphoreDelete(semaphore);
                    return false;
                }

                tokenAvailable = false;
            }
        }

        vSemaphoreDelete(semaphore);
        Log::sys_info(kTag, "Random semaphore interactions successful");
        return true;
    }
} // namespace UnitTest
