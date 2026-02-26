#include "./port_layer.h"

#include "./context_switch_asm.h"
#include "./interrupt_vector_mapping.h"
#include "./register_saving.h"
#include "./systick_configuration.h"

#include "../../../../serial/log.h"

namespace UnitTest
{
    bool runRtosPortLayerSuite()
    {
        constexpr const char *kTag = "RTOS_PORT";

        bool ok = true;
        ok = runRtosPortRegisterSavingTest() && ok;
        ok = runRtosPortInterruptVectorMappingTest() && ok;
        ok = runRtosPortContextSwitchAsmCorrectnessTest() && ok;
        ok = runRtosPortSysTickConfigurationTest() && ok;

        if (!ok)
        {
            Log::sys_error(kTag, "One or more port layer tests failed");
            return false;
        }

        Log::sys_info(kTag, "Port layer tests successful");
        return true;
    }
} // namespace UnitTest
