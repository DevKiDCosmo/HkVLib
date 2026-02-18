#pragma once

#include <Arduino.h>
#include <vector>

/**
 * @brief State data structure for saving/restoring process state
 */
struct ProcessState
{
    uint32_t operation_id;
    unsigned long timestamp;
    uint8_t priority;
    bool is_critical;
};

/**
 * @brief OnlineLock class - Manages network availability lock with interrupt and state management
 *
 * Features:
 * - Monitors WiFi connectivity
 * - Engages lock when offline (interrupts running operations)
 * - Saves process state before interrupt
 * - Resumes process when connection restored
 */
class OnlineLock
{
public:
    /**
     * @brief Initialize the online lock system
     */
    static void init();

    /**
     * @brief Check if online lock is currently active (offline mode)
     * @return true if locked (offline), false if unlocked (online)
     */
    static bool isLocked();

    /**
     * @brief Manually engage the lock (trigger offline mode)
     */
    static void engageLock();

    /**
     * @brief Manually disengage the lock (trigger online mode)
     */
    static void disengageLock();

    /**
     * @brief Save current process state before interruption
     * @param op_id Operation ID to save
     * @param is_critical Whether operation is critical (affects resume behavior)
     */
    static void saveProcessState(uint32_t op_id, bool is_critical = false);

    /**
     * @brief Check if there's saved state to resume
     * @return true if saved state exists
     */
    static bool hasSavedState();

    /**
     * @brief Get saved process state
     * @return ProcessState structure with saved data
     */
    static ProcessState getSavedState();

    /**
     * @brief Clear saved state after processing
     */
    static void clearSavedState();

    /**
     * @brief Get lock status change event (true = just transitioned)
     * @return true if lock status just changed
     */
    static bool statusChanged();

private:
    static bool locked;
    static bool status_changed;
    static ProcessState saved_state;
    static bool state_saved;
};
