Still in production. Pwr are not truly done.

Good pratice: Fast ut and not slow ut.

Unit testing an **RTOS (Real-Time Operating System)** [ ] is fundamentally different from normal library testing because you must verify:

* [x] Determinism
* [x] Timing guarantees
* [x] Concurrency correctness
* [x] Interrupt behavior
* [x] Memory safety under real-time constraints

Below is a structured and complete RTOS test matrix used in industrial embedded systems.

## 1. Scheduler Tests (Core of RTOS)

### 1.1 Priority Scheduling Test

* [x] Higher priority task always preempts lower priority
* [x] Equal priority → round robin?
* [x] Starvation detection

### 1.2 Preemption Test

* [x] Task A running
* [x] Higher priority task B becomes ready
* [x] Immediate context switch?

### 1.3 Time Slice / Round Robin

* [x] Time quantum respected?
* [x] Context switch occurs exactly after tick?

### 1.4 Context Switch Integrity

* [x] Registers preserved?
* [x] Stack pointer correct?
* [x] FPU state saved/restored?
* [x] No corruption after 10k switches

## 2. Timing Tests (Real-Time Guarantees)

### 2.1 Task Execution Deadline

* [x] Task finishes before deadline
* [x] Deadline miss detection

### 2.2 Tick Accuracy

* [x] System tick frequency correct?
* [x] Drift over long runtime?

### 2.3 Jitter Measurement

* [x] ISR latency jitter
* [x] Task wake-up jitter

### 2.4 Worst Case Execution Time (WCET)

* [x] Measure upper bound under load

## 3. Interrupt Handling Tests

### 3.1 ISR Preemption

* [x] ISR interrupts running task?
* [x] Nested interrupts working?

### 3.2 Interrupt Latency

* [x] Time from interrupt signal → handler execution

### 3.3 ISR to Task Signaling

* [x] Semaphore from ISR?
* [x] Queue send from ISR?

### 3.4 Disable/Enable Interrupt Safety

* [x] Critical section protection
* [x] No missed interrupts

## 4. Synchronization Primitive Tests

### 4.1 Mutex Tests

* [x] Lock/Unlock correctness
* [x] Priority inheritance works?
* [x] Deadlock detection?

### 4.2 Semaphore Tests

* [x] Binary semaphore behavior
* [x] Counting semaphore limits
* [x] ISR-safe usage

### 4.3 Event Flags

* [x] Multiple tasks waiting?
* [x] Bitwise logic correct?

### 4.4 Condition Variables (if supported)

* [x] Wake exactly correct task(s)

## 5. Deadlock & Starvation Tests

* [x] Circular wait scenario
* [x] Priority inversion
* [x] Long-term starvation simulation

## 6. Memory Management Tests

### 6.1 Static Allocation

* [x] Stack overflow detection
* [x] Task stack boundary checking

### 6.2 Dynamic Allocation

* [x] Heap fragmentation test
* [x] Allocation under stress
* [x] Deterministic allocation time?

### 6.3 Memory Protection (if MPU)

* [x] Task isolation
* [x] Illegal access trap

## 7. Inter-Task Communication (IPC)

### 7.1 Queue Tests

* [x] FIFO behavior
* [x] Overflow handling
* [x] Blocking behavior

### 7.2 Mailbox Tests

* [x] Single-slot overwrite?
* [x] Blocking semantics

### 7.3 Shared Memory

* [x] Race detection
* [x] Data consistency

## 8. Power Management Tests

* [x] Tickless idle
* [x] Sleep mode entry/exit
* [x] Wake-up source correctness
* [x] Timekeeping after sleep

## 9. Multi-Core / SMP Tests (if supported)

* [x] Task migration
* [x] Load balancing
* [x] Inter-core interrupt
* [x] Cross-core synchronization

## 10. Fault Injection Tests

* [x] Simulated ISR storm
* [x] Forced memory allocation fail
* [x] Forced context switch failure
* [x] Artificial delay injection

## 11. Stress Tests

* [ ] 100+ tasks
* [ ] Rapid create/delete
* [ ] Maximum queue usage
* [ ] Continuous ISR triggering

## 12. Determinism Tests

Very important.

Same inputs → same schedule behavior?
Same timing under repeated execution?
No nondeterministic behavior?

## 13. Safety-Critical Tests (Industrial Level)

Used in automotive / aerospace.

* [ ] Stack overflow hook works?
* [ ] Watchdog integration?
* [ ] Safe state on fatal error?
* [ ] Brown-out detection?

## 14. Port Layer Tests (Hardware Abstraction)

* [ ] Correct register saving
* [ ] Correct interrupt vector mapping
* [ ] Context switch ASM correctness
* [ ] SysTick configuration

## 15. Real Hardware vs Simulation Tests

You need both:

### On Host (Mocked)

* [ ] Scheduler logic
* [ ] IPC logic
* [ ] Memory allocator

### On Target Hardware

* [ ] Timing precision
* [ ] Interrupt latency
* [ ] Power behavior

## 16. Certification-Level Tests

For:

* [ ] ISO 26262
* [ ] DO-178C
* [ ] IEC 61508

You need:

* [ ] Traceability
* [ ] Requirement-to-test mapping
* [ ] Code coverage (MC/DC)
* [ ] Structural coverage

## 17. Coverage Tests

* [ ] Branch coverage
* [ ] Path coverage
* [ ] ISR coverage
* [ ] Error branch coverage

## 18. Fuzz Testing for RTOS

Yes, even RTOS.

* [ ] Random task creation patterns
* [ ] Random interrupt timing
* [ ] Random semaphore interactions

## 19. Example Minimal RTOS Test Set (Practical)

If you want a solid baseline:

1. Scheduler preemption test
2. Context switch integrity test
3. Semaphore correctness test
4. Mutex priority inheritance test
5. ISR latency measurement
6. Stack overflow detection test
7. Queue overflow test
8. Memory fragmentation stress test
9. Deadlock scenario test
10. Long-run stability test (24h loop)

## 20. Advanced: Formal Verification

For your level (compiler/OS design):

* [ ] Prove no deadlock in scheduler
* [ ] Prove bounded interrupt latency
* [ ] Prove priority inheritance correctness
* [ ] Model check scheduling

* [ ] Static or dynamic memory?
* [ ] Safety-critical target?
