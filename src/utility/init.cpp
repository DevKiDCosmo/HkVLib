#include "init.h"
#include "../serial/log.h"

static int INIT_ = 0;
static bool initialized_ = false;
static bool changable = true;

void Init::initialized()
{
    if (initialized_)
    {
        Log::sys_warning("INIT", "Already initialized, skipping!");
        return;
    }
    INIT_ = 1;
    initialized_ = true;
    changable = false;
    Log::sys_info("INIT", "Initialization complete");
}

int Init::value()
{
    return INIT_;
}