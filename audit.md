# RTOS Unit-Test Audit

Datum: 2026-05-10
Scope: src/unittest/components/RTOS
Ziel: Verbesserungen priorisieren und fehlende Tests fuer einen vollstaendigen, robusten Unit-Test-Umfang definieren.

## Executive Summary

Der RTOS-Testbaum deckt viele Themenbereiche ab (Scheduler, Timing, Interrupts, Sync, IPC, Safety, SMP, Fuzz, Coverage). Die groessten Qualitaetsluecken liegen jedoch in:

1. Proxy-/Delegationstests statt eigenstaendiger Verifikation in mehreren Schichten.
2. Unvollstaendige Negativ- und Boundary-Testfaelle.
3. Harte, nicht kalibrierte Timing-Schwellwerte (flake-anfellig auf anderer Last/Hardware).
4. Teilweise semantisch zu schwache Tests (z. B. Circular-Wait ohne echten zyklischen Wait).

Prioritaet: Erst Testvaliditaet und Determinismus-Methodik stabilisieren, danach Abdeckung verbreitern.

## A. Priorisierte Findings und Verbesserungsvorschlaege

### A1. Hoch: Proxy-Tests statt eigener Evidenz

Problem:
Mehrere Tests delegieren nur auf bestehende Tests. Dadurch entsteht nominelle Abdeckung, aber keine eigene Aussagekraft je Schicht.

Fundstellen:
- src/unittest/components/RTOS/formal_verification/prove_bounded_interrupt_latency.cpp:7
- src/unittest/components/RTOS/port_layer/interrupt_vector_mapping.cpp:7
- src/unittest/components/RTOS/hardware_simulation/target_timing_precision.cpp:7
- src/unittest/components/RTOS/stress/continuous_isr_triggering.cpp:7

Vorschlag:
- Jede Schicht braucht eigene Assertions und Metriken.
- Formal-Verifikation: Invarianten explizit pruefen (nicht nur runRtosInterruptLatencyTest aufrufen).
- Port-Layer: Port-spezifische Vektor-/Registerannahmen pruefen.
- Hardware-Simulation: Host-vs-Target-Vergleich mit eigenen Kriterien.
- Stress: echte Dauer-/Lastprofile statt Alias-Wrapper.

### A2. Hoch: Circular-Wait-Test bildet kein echtes Circular-Wait-Szenario ab

Problem:
Aktuell lockt ein einzelner Task Mutex A und B nacheinander. Das ist kein zyklischer Wait zwischen mindestens zwei Tasks.

Fundstelle:
- src/unittest/components/RTOS/deadlock_starvation/circular_wait.cpp:30

Vorschlag:
- Zwei Tasks mit gegenlaeufiger Reihenfolge (A->B und B->A).
- Kontrollierter Synchronisationspunkt, danach Timeout- und Recovery-Pfad.
- Explizite Deadlock-Detektion inklusive erwarteter Fehlermeldung.

### A3. Hoch: Integer-Overflow-Risiko im Forced-Allocation-Fail-Test

Problem:
freeInternal + 1024 kann ueberlaufen.

Fundstelle:
- src/unittest/components/RTOS/fault_injection/memory_allocation_fail.cpp:15

Vorschlag:
- Overflow-sichere Addition mit Guard.
- Zusaetzliche Grenzfalltests (nahe SIZE_MAX).

### A4. Hoch: Harte Timing-Grenzen ohne Kalibrierung

Problem:
Fixe Schwellwerte sind nicht robust ueber Build-Profile, Last und Hardware-Varianten.

Fundstellen:
- src/unittest/components/RTOS/interrupts/interrupt_latency.cpp:16
- src/unittest/components/RTOS/timing/deadline.cpp:16
- src/unittest/components/RTOS/scheduler/preemption.cpp:49
- src/unittest/components/RTOS/determinism/schedule_repeatability.cpp:43
- src/unittest/components/RTOS/determinism/timing_repeatability.cpp:31

Vorschlag:
- Warmup und Baseline-Messung.
- Schwellwerte aus Perzentilen (z. B. P95/P99) ableiten.
- Profile fuer Debug/Release und Board-Klassen getrennt pflegen.

### A5. Mittel: Static-Allocation-Test testet nicht wirklich Static Allocation

Problem:
Prueft nur Stack High Water Mark des aktuellen Tasks, nicht xTaskCreateStatic-Pfad.

Fundstelle:
- src/unittest/components/RTOS/memory_management/static_allocation.cpp:13

Vorschlag:
- Echte static task/control block Buffer nutzen.
- Verifizieren, dass kein Heap-Pfad genutzt wird.

### A6. Mittel: ISR-Storm ist nur Critical-Section-Toggle

Problem:
Kein echter ISR-Druck, keine Handler-/Queue-Ueberlastung, keine Recovery-Metriken.

Fundstelle:
- src/unittest/components/RTOS/fault_injection/isr_storm.cpp:17

Vorschlag:
- Echte ISR-Quelle (Timer/GPIO) simulieren oder triggern.
- Backlog, Latenz und Verlustquote messen.

### A7. Mittel: Condition-Variable-Ersatz mit fragilem Handshake

Problem:
Handle-Readiness und Signalisierung sind eng gekoppelt und potenziell fragil.

Fundstellen:
- src/unittest/components/RTOS/synchronization/condition_variables.cpp:21
- src/unittest/components/RTOS/synchronization/condition_variables.cpp:52

Vorschlag:
- Handshake explizit serialisieren (z. B. zweistufige Notification/Atomic-Status).
- Negative Faelle (timeout/no-signal) getrennt validieren.

### A8. Niedrig: Coverage-Tests teils toy-level statt RTOS-nah

Problem:
Lokale Pfadfunktion statt realer RTOS-Codepfade reduziert Aussagekraft.

Fundstelle:
- src/unittest/components/RTOS/coverage/path_coverage.cpp:9

Vorschlag:
- Branch/Path-Coverage auf produktionsnahe RTOS-Helferpfade lenken.

### A9. Niedrig: Safety-Test prueft Konfiguration statt Verhalten

Problem:
configCHECK_FOR_STACK_OVERFLOW wird abgefragt, aber Hook-Verhalten nicht testgetrieben verifiziert.

Fundstelle:
- src/unittest/components/RTOS/safety_critical/stack_overflow_hook.cpp:11

Vorschlag:
- Kontrollierten Hook-Trigger/Mock einfuehren.
- Erwarteten Safe-State-Pfad pruefen.

### A10. Niedrig: Top-Level Runner ohne feingranulares Ergebnisobjekt

Problem:
Nur bool Aggregation; keine standardisierte Testmetrik pro Suite (Dauer, Kategorie, Fehlercode).

Fundstelle:
- src/unittest/components/RTOS/RTOS.cpp:69

Vorschlag:
- Result-Struktur (name, status, duration_ms, error_code, flaky_hint).
- Konsolidiertes Audit-Log pro Lauf.

## B. Fehlende Testbausteine fuer einen vollstaendigen Unit-Test-Umfang

### B1. Negativtest-Matrix pro Modul

Fehlt aktuell (systematisch):
- NULL-Handles
- Invalid Priority
- Zero timeout vs finite timeout
- Invalid sequence (double give/delete)
- Empty/overflow conditions bei IPC

Soll:
- Pro API mindestens 3 Negativfaelle.
- Erwartete Fehlerpfade und Rueckgabecodes explizit assertieren.

### B2. Boundary-/Extremwerttests

Fehlt aktuell:
- Minimal-/Maximal-Stack
- Queue fast full/full transitions
- Tick-Wraparound-Szenarien
- Task creation near system limits

Soll:
- Jede Primitive mit min/typ/max Profil pruefen.

### B3. Determinismus-Statistik statt Einzelgrenze

Fehlt aktuell:
- Groessere Sample-Mengen
- Streuungsmetriken
- Wiederholbarkeit ueber Runs

Soll:
- >=100 Iterationen bei Timing-Tests.
- P50/P95/P99 + max ausgeben.
- Regressionsschwellen datengetrieben definieren.

### B4. Reproduzierbarkeit in Fuzz/Fault Injection

Fehlt aktuell:
- Seed-Logging und deterministische Wiederholung
- Definierte Seed-Sets fuer CI

Soll:
- Seed immer loggen.
- Failures mit Seed reproduzierbar machen.

### B5. Leak/Isolation-Pruefung pro Test

Fehlt aktuell:
- Standardisierter Vorher/Nachher-Check auf Ressourcen.

Soll:
- Nach jedem Test: Heap, Task count, Queue/Semaphore count validieren.
- Test darf keine Side-Effects hinterlassen.

### B6. Orchestrierung und Reporting

Fehlt aktuell:
- Einheitliche Test-ID, Kategorie, Timing im Report.

Soll:
- Strukturierter Report fuer lokale Analyse und CI.
- Flaky-Detection-Flag je Test.

### B7. Host-vs-Target Paritaet

Fehlt aktuell:
- Echte semantische Differenzpruefung Host vs Target.

Soll:
- Gleiches Erwartungsmodell mit schichtspezifischen Abweichungsfenstern.

## C. Konkreter Test-Backlog (vorgeschlagen)

### C1. Must-Have (P0)

1. deadlock_circular_wait_two_tasks_timeout
2. allocation_fail_overflow_guard
3. static_allocation_true_static_path
4. interrupt_latency_profiled_thresholds
5. preemption_under_background_load
6. timing_repeatability_percentile_based
7. condition_var_handshake_timeout_negative
8. queue_null_handle_negative
9. semaphore_double_give_negative
10. task_create_invalid_priority_negative

### C2. Should-Have (P1)

11. tick_wraparound_behavior
12. queue_full_then_receive_transition
13. mailbox_overwrite_race_case
14. event_group_partial_bits_timeout
15. mutex_priority_inheritance_stress
16. isr_storm_real_timer_source
17. watchdog_recovery_path_mock
18. safe_state_on_fatal_error_transition
19. power_tickless_resume_time_drift
20. smp_migration_with_contention

### C3. Nice-to-Have (P2)

21. fuzz_seed_replay_smoke_set
22. randomized_ipc_with_invariant_checks
23. branch_coverage_on_rtos_wrappers
24. formal_invariant_documented_asserts
25. host_target_parity_smoke
26. stress_long_run_memory_fragmentation
27. context_switch_integrity_extended_register_set
28. interrupt_disable_enable_nested_levels
29. brownout_simulated_recovery_path
30. deterministic_schedule_under_mixed_priority_load

## D. Qualitaetskriterien (Definition of Done)

Ein Testbereich gilt als "vollstaendig genug" wenn:

1. Positiv + Negativ + Boundary vorhanden sind.
2. Zeitbezogene Assertions datenbasiert und profilbewusst sind.
3. Ressourcen nach Testlauf vollstaendig freigegeben sind.
4. Ergebnisse reproduzierbar sind (Seeds, Metriken, Konfiguration).
5. Testreports feingranular und CI-tauglich sind.

## E. Umsetzungs-Roadmap

Phase 1 (Sofort, 1-2 Tage):
- P0 Tests 1-5 umsetzen
- Proxy-Tests markieren und mit TODO-Label fuer echte Assertions ersetzen
- Overflow-Guard integrieren

Phase 2 (Kurzfristig, 3-5 Tage):
- Restliche P0 + P1 Kernfaelle
- Timing-Profile und Perzentil-Auswertung
- Standardisierte Result-Struktur im Runner

Phase 3 (Mittelfristig, 1-2 Wochen):
- P2 Erweiterungen
- Host-vs-Target Paritaetslayer
- Flaky-Analyse in CI

## F. Optionaler Implementierungsstart (Top 3)

Wenn direkt Codeanpassungen gewuenscht sind, zuerst:

1. src/unittest/components/RTOS/deadlock_starvation/circular_wait.cpp
2. src/unittest/components/RTOS/memory_management/static_allocation.cpp
3. src/unittest/components/RTOS/interrupts/interrupt_latency.cpp

Diese drei liefern den groessten Hebel fuer Validitaet und Stabilitaet.
