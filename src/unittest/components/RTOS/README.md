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

* [ ] Task finishes before deadline
* [ ] Deadline miss detection

### 2.2 Tick Accuracy

* [ ] System tick frequency correct?
* [ ] Drift over long runtime?

### 2.3 Jitter Measurement

* [ ] ISR latency jitter
* [ ] Task wake-up jitter

### 2.4 Worst Case Execution Time (WCET)

* [ ] Measure upper bound under load

## 3. Interrupt Handling Tests

### 3.1 ISR Preemption

* [ ] ISR interrupts running task?
* [ ] Nested interrupts working?

### 3.2 Interrupt Latency

* [ ] Time from interrupt signal → handler execution

### 3.3 ISR to Task Signaling

* [ ] Semaphore from ISR?
* [ ] Queue send from ISR?

### 3.4 Disable/Enable Interrupt Safety

* [ ] Critical section protection
* [ ] No missed interrupts

## 4. Synchronization Primitive Tests

### 4.1 Mutex Tests

* [ ] Lock/Unlock correctness
* [ ] Priority inheritance works?
* [ ] Deadlock detection?

### 4.2 Semaphore Tests

* [ ] Binary semaphore behavior
* [ ] Counting semaphore limits
* [ ] ISR-safe usage

### 4.3 Event Flags

* [ ] Multiple tasks waiting?
* [ ] Bitwise logic correct?

### 4.4 Condition Variables (if supported)

* [ ] Wake exactly correct task(s)

## 5. Deadlock & Starvation Tests

* [ ] Circular wait scenario
* [ ] Priority inversion
* [ ] Long-term starvation simulation

## 6. Memory Management Tests

### 6.1 Static Allocation

* [ ] Stack overflow detection
* [ ] Task stack boundary checking

### 6.2 Dynamic Allocation

* [ ] Heap fragmentation test
* [ ] Allocation under stress
* [ ] Deterministic allocation time?

### 6.3 Memory Protection (if MPU)

* [ ] Task isolation
* [ ] Illegal access trap

## 7. Inter-Task Communication (IPC)

### 7.1 Queue Tests

* [ ] FIFO behavior
* [ ] Overflow handling
* [ ] Blocking behavior

### 7.2 Mailbox Tests

* [ ] Single-slot overwrite?
* [ ] Blocking semantics

### 7.3 Shared Memory

* [ ] Race detection
* [ ] Data consistency

## 8. Power Management Tests

* [ ] Tickless idle
* [ ] Sleep mode entry/exit
* [ ] Wake-up source correctness
* [ ] Timekeeping after sleep

## 9. Multi-Core / SMP Tests (if supported)

* [ ] Task migration
* [ ] Load balancing
* [ ] Inter-core interrupt
* [ ] Cross-core synchronization

## 10. Fault Injection Tests

* [ ] Simulated ISR storm
* [ ] Forced memory allocation fail
* [ ] Forced context switch failure
* [ ] Artificial delay injection

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
