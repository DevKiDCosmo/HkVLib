#pragma once

/**
 * @brief Daemon class providing static methods to start all background daemon tasks.
 *
 * All daemons run as FreeRTOS tasks pinned to specific cores.
 */
class Daemon
{
public:
    /**
     * @brief Start the network daemon (WiFi monitoring and reconnection).
     * Runs on Core 0 with 4096 byte stack.
     */
    static void startNetworkDaemon(void);

    /**
     * @brief Start the heartbeat daemon (periodic server ping).
     * Runs on Core 1 with 8192 byte stack.
     */
    static void startHeartbeatDaemon(void);

    /**
     *
     */
    static void startSerialInputDaemon(void);
};
