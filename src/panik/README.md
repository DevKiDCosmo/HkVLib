# Interrupt + Panic + Critical Scope + Freeze System
## Enhanced Locking and Runtime Control Architecture for RTOS Daemons
*(Architecture Draft – Unified Document)*

---

# 1. Purpose

This system extends an RTOS-based firmware with:

- Immediate cooperative interruption
- Deterministic panic handling
- Critical execution scopes (atomic completion guarantee)
- Global freeze mode
- Enhanced lock tracking
- Safe resume and recovery

Design goal:

Deterministic. Interruptible. Safe. No inconsistent mutex states. No unsafe task suspension.

---

# 2. System State Model

```c
typedef enum {
    SYS_RUN,
    SYS_PAUSE_REQUESTED,
    SYS_PAUSED,
    SYS_FREEZE,
    SYS_PANIC,
    SYS_RECOVERY
} system_state_t;
```

```c
volatile system_state_t g_system_state = SYS_RUN;
```

---

# 3. Core Principles

1. No external `vTaskSuspend()`
2. No forced task stack freezing
3. All interruption is cooperative
4. Atomic sections must complete
5. Panic overrides everything
6. Freeze affects all tasks except recovery handlers

---

# 4. Critical Scope System

## 4.1 Purpose

A **Critical Scope** guarantees:

* Execution must complete
* No interruption allowed
* No blocking allowed
* Very short duration

## 4.2 Scope Depth Counter (Nested Safe)

```c
volatile uint32_t g_critical_depth = 0;
```

## 4.3 Critical Scope Macros

```c
#define CRITICAL_SCOPE_BEGIN()        \
    taskENTER_CRITICAL();             \
    g_critical_depth++;

#define CRITICAL_SCOPE_END()          \
    g_critical_depth--;               \
    taskEXIT_CRITICAL();
```

Rules:

* No `vTaskDelay()`
* No semaphore wait
* No long loops
* Only atomic memory or peripheral operations

---

# 5. Cooperative Pause Mechanism

Pause is allowed only outside critical scopes.

```c
void cooperative_pause_point(void)
{
    if (g_system_state == SYS_PAUSE_REQUESTED &&
        g_critical_depth == 0)
    {
        g_system_state = SYS_PAUSED;

        while (g_system_state == SYS_PAUSED)
        {
            vTaskDelay(1);
        }
    }
}
```

Task Integration:

```c
while (1)
{
    CRITICAL_SCOPE_BEGIN();
    atomic_update();
    CRITICAL_SCOPE_END();

    cooperative_pause_point();
}
```

---

# 6. Freeze System

## 6.1 Concept

Freeze = Global execution halt.

Difference from Pause:

* Pause = selective control
* Freeze = global stop
* Only panic/recovery logic continues

## 6.2 Trigger

```c
void system_freeze(void)
{
    g_system_state = SYS_FREEZE;
}
```

## 6.3 Freeze Check

```c
void freeze_check(void)
{
    if (g_system_state == SYS_FREEZE &&
        g_critical_depth == 0)
    {
        while (g_system_state == SYS_FREEZE)
        {
            __asm volatile("nop");
        }
    }
}
```

Task Integration:

```c
while (1)
{
    freeze_check();
    panic_check();
    do_work();
}
```

Freeze never interrupts inside a Critical Scope.

---

# 7. Panic System

## 7.1 Purpose

Panic is an absolute deterministic stop.

Triggered by:

* Watchdog
* Fault ISR
* Assertion failure
* Hardware error

## 7.2 Trigger

```c
void system_trigger_panic(void)
{
    taskENTER_CRITICAL();
    g_system_state = SYS_PANIC;
}
```

## 7.3 Panic Check

```c
void panic_check(void)
{
    if (g_system_state == SYS_PANIC)
    {
        shutdown_peripherals();
        release_all_locks();
        store_fault_info();

        while (1)
        {
            // Optional: system reset
        }
    }
}
```

Panic overrides:

* Pause
* Freeze
* Normal scheduling

---

# 8. Enhanced Lock System

## 8.1 Lock Structure

```c
typedef struct {
    SemaphoreHandle_t mutex;
    uint8_t locked;
    uint8_t owner_id;
} enhanced_lock_t;
```

## 8.2 Acquire

```c
void enhanced_lock_acquire(enhanced_lock_t *lock, uint8_t id)
{
    xSemaphoreTake(lock->mutex, portMAX_DELAY);
    lock->locked = 1;
    lock->owner_id = id;
}
```

## 8.3 Release

```c
void enhanced_lock_release(enhanced_lock_t *lock)
{
    lock->locked = 0;
    lock->owner_id = 0;
    xSemaphoreGive(lock->mutex);
}
```

## 8.4 Panic Lock Sweep

```c
void release_all_locks(void)
{
    for (int i = 0; i < LOCK_COUNT; i++)
    {
        if (lock_table[i].locked)
        {
            xSemaphoreGive(lock_table[i].mutex);
            lock_table[i].locked = 0;
        }
    }
}
```

Freeze does NOT unlock.
Panic forces global unlock.

---

# 9. Execution Flow Model

## Normal Operation

```
RUN
 ↓
CRITICAL_SCOPE
 ↓
PAUSE_POINT
```

## Pause Transition

```
RUN → PAUSE_REQUESTED → PAUSED → RUN
```

## Freeze Transition

```
RUN → FREEZE → RUN
```

## Panic Transition

```
ANY_STATE → PANIC → SHUTDOWN
```

---

# 10. Latency Characteristics

| Mechanism | Reaction Time                |
| --------- | ---------------------------- |
| Pause     | Next pause point             |
| Freeze    | After exiting Critical Scope |
| Panic     | Next panic check             |
| ISR Panic | Near immediate               |

---

# 11. Task Template

```c
void daemon_task(void *arg)
{
    while (1)
    {
        freeze_check();
        panic_check();

        CRITICAL_SCOPE_BEGIN();
        critical_update();
        CRITICAL_SCOPE_END();

        cooperative_pause_point();

        process_next_step();
    }
}
```

---

# 12. Safety Properties

* No stack corruption
* No mid-mutex suspension
* No unsafe task kill
* Deterministic state transitions
* RTOS compliant
* Safety-expandable

---

# 13. Concept Summary

Interrupt = cooperative interruption
Critical Scope = atomic execution guarantee
Freeze = global controlled halt
Panic = deterministic hard stop

---

# 14. Architectural Goals

* Real-time compatible
* Lock-safe
* Deterministic recovery
* Scalable across daemons
* Compatible with any preemptive RTOS

