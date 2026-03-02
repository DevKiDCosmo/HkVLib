#pragma once

#include "../../include.h"

namespace SerialCommandRegistry
{
    bool dispatch(const String &input);
    void printHelp();
}
