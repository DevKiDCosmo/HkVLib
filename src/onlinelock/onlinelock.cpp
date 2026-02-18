#include "onlinelock.h"
#include "../serial/log.h"

static const char *LOCK_TAG = "ONLINE_LOCK";

// Static member initialization
bool OnlineLock::locked = false;
bool OnlineLock::status_changed = false;
ProcessState OnlineLock::saved_state = {0, 0, 0, false};
bool OnlineLock::state_saved = false;

void OnlineLock::init()
{
    locked = false;
    status_changed = false;
    state_saved = false;
    Log::sys_info(LOCK_TAG, "OnlineLock initialized");
}

bool OnlineLock::isLocked()
{
    return locked;
}

void OnlineLock::engageLock()
{
    if (!locked)
    {
        locked = true;
        status_changed = true;
        Log::sys_warning(LOCK_TAG, "ONLINE LOCK ENGAGED - Operations interrupted!");
    }
}

void OnlineLock::disengageLock()
{
    if (locked)
    {
        locked = false;
        status_changed = true;
        Log::sys_info(LOCK_TAG, "ONLINE LOCK DISENGAGED - Resuming operations...");
    }
}

void OnlineLock::saveProcessState(uint32_t op_id, bool is_critical)
{
    saved_state.operation_id = op_id;
    saved_state.timestamp = millis();
    saved_state.is_critical = is_critical;
    saved_state.priority = is_critical ? 2 : 1;
    state_saved = true;

    Log::sys_info(LOCK_TAG, "Process state saved - Op ID: " + String(op_id) + ", Critical: " + (is_critical ? "YES" : "NO"));
}

bool OnlineLock::hasSavedState()
{
    return state_saved;
}

ProcessState OnlineLock::getSavedState()
{
    return saved_state;
}

void OnlineLock::clearSavedState()
{
    state_saved = false;
    saved_state = {0, 0, 0, false};
    Log::sys_info(LOCK_TAG, "Saved state cleared");
}

bool OnlineLock::statusChanged()
{
    bool changed = status_changed;
    status_changed = false;
    return changed;
}
