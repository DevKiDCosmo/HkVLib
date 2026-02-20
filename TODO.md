# TODO (Codebase-Scan)

Stand: automatisch aus dem aktuellen Repo-Status abgeleitet.

## P0 – Build-/Integrationsblocker

- [ ] `main.cpp` Warning/Fix: `create_text(L"...")` bekommt `wchar_t*` statt `const wchar_t*` (aktueller Compiler-Warning in `src/main.cpp`).
- [ ] Entscheiden, ob BLE/P2P aktiv werden soll: aktuelle Implementierungen liegen nur als `.ignore/.ignoreh` vor und sind nicht im aktiven Build eingebunden.

## P1 – Netzwerk & Connectivity

### P2P + BLE
- [ ] `src/connectivity/bluetooth/bluetooth.ignore` / `.ignoreh` in echte `bluetooth.cpp/.h` überführen.
- [ ] `src/p2p/protocol.ignore` / `.ignoreh` in echte `protocol.cpp/.h` überführen.
- [ ] Danach Includes/Module verdrahten (aktuell keine aktiven Includes auf `bluetooth.h`/`protocol.h` im Build).
- [ ] P2P/BLE Integrationspfad in Daemons oder Missionslogik definieren (derzeit keine Runtime-Nutzung).

### WiFi
- [ ] `WiFiConnect::getPingLatency()` implementieren (aktuell Placeholder `return 0` in `src/connectivity/wifi/wifi.cpp`).

## P1 – Daemons

- [ ] `src/daemon/health/healthDaemon.cpp` ist leer → Health-Logik ergänzen oder Datei entfernen.
- [ ] `src/daemon/health/sensorDaemon.cpp` ist leer → Sensor-Health-Daemon implementieren oder Datei entfernen.
- [ ] Daemon-Startstrategie für Health-Subdaemons klarziehen (nur `HealthWiFiDaemon` aktiv).

## P1 – UnitTest

- [ ] `src/unittest/components/RTOS/RTOS.cpp` ist leer.
- [ ] `src/unittest/components/RTOS/RTOS.h` ist leer.
- [ ] RTOS-Testplan aus `src/unittest/components/RTOS/README.md` in echte Tests überführen.
- [ ] PRIO-0 Tests aus `src/unittest/README.md` vervollständigen (RTOS, Daemon-Verifikation, Calibration, CyberPI Bridge, Network, I2C/SPI, GPIO, Watchdog, Clock, Interrupts, Boot Partition, Panic System).
- [ ] PRIO-1 Tests aus `src/unittest/README.md` aufbauen (WiFi/Bluetooth/MQTT/P2P/Display/Identity/OnlineLock/Mission/Config etc.).

## P2 – Serial Command Palette

- [ ] Aktuell nur `wifi status` und `reboot` in `src/daemon/serialInputDaemon.cpp`.
- [ ] Kommando-Registry/Parser einführen (statt langer `if/else`).
- [ ] Mindestset ergänzen: `help`, `health`, `loglevel`, `wifi reconnect`, `daemon status`, `display clear`, `uptime`, `heap`.
- [ ] Unbekannte Befehle + Syntax-Hilfe sauber zurückmelden.

## P2 – Display (Enhance Display)

- [ ] `Display::initialize/render/update/shutdown` sind aktuell leer in `src/display/display.cpp`.
- [ ] Log-Rendering erweitern (z. B. Level-Farben INFO/WARN/ERROR über `PresetColor`).
- [ ] Boot-/Log-Anzeige konsistent in den Init-Flow integrieren (Doppel-Draw in `main.cpp` bereinigen).
- [ ] Optional: API für beliebige kompilierte Bitmaps dokumentieren (Generator vorhanden: `tools/bitmap_to_rgb565.py`).

## P2 – Konfiguration & Struktur

- [ ] `src/config/global.h` ist leer.
- [ ] `src/config.yml` ist leer.
- [ ] Entscheiden: entfernen oder mit echter Konfiguration befüllen.

## P3 – Server/Microservice

- [ ] `server/microservice/dhcpid.py`: TODO umsetzen („Asynchron task that deletes old bots“).

## P3 – Test-/Build-Infrastruktur

- [ ] Passive Build-Time-Checks aus `src/unittest/README.md` schrittweise implementieren.


Add time out for Storage test.