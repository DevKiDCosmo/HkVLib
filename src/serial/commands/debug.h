#pragma once

#include "../../include.h"

class SerialDebugCommands
{
public:
    static void RTOSBgTask();
    static void DisplayPing();
    static void DaemonNotify(const String &daemonTask);

private:
};