# Serial Commands

Diese Datei dokumentiert die Command-Registry und alle verfügbaren Serial-Commands.

## Status

- [x] Command-Registry mit Map aktiv
- [x] Zentraler Dispatcher (`SerialCommandRegistry::dispatch`)
- [x] Help-Ausgabe aus Registry-Metadaten

## Registry Design

- [x] Pro Command werden gespeichert: `command`, `description`, `usage`, `acceptsArgs`, `handler`
- [x] Neue Commands werden über `registerCommand(...)` ergänzt
- [x] Parser-Komplexität reduziert (keine lange `if/else`-Kette mehr)

## Commands (registriert)

### System

- [x] `help` — Liste aller Commands
- [x] `version` — Firmware-Version/Build/Date
- [x] `uptime` — Uptime in ms
- [x] `heap status` — Heap-Übersicht
- [x] `boot reason` — Reset-Grund
- [x] `reboot` — Sofortiger Reboot

### App

- [x] `app start` — Placeholder (not implemented)
- [x] `app stop` — Placeholder (not implemented)
- [x] `app restart` — Placeholder (not implemented)
- [x] `app state` — Placeholder (not implemented)
- [x] `app upload` — Placeholder (not implemented)

### WiFi

- [x] `wifi status` — Verbindungsstatus + IP
- [x] `wifi scan` — SSID-Scan
- [x] `wifi connect <ssid> <pw>` — Verbinden
- [x] `wifi disconnect` — Trennen
- [x] `wifi ip` — IP anzeigen
- [x] `wifi reconnect` — Reconnect mit Cache-Credentials

### Network

- [x] `net ping <host>` — Placeholder (not implemented)
- [x] `net dns <host>` — Placeholder (not implemented)
- [x] `net time` — Placeholder (not implemented)
- [x] `net req <url>` — Placeholder (not implemented)
- [x] `net stats` — Placeholder (not implemented)

### Daemons & Debug

- [x] `rtos tasks` — Task/Daemon-Übersicht
- [x] `daemon status` — Alias für `rtos tasks`
- [x] `daemon list` — Alias für Task-Übersicht
- [x] `daemon notify <taskname>` — Watcher bis Task wieder `Running`
- [x] `daemon start <name>` — Placeholder (not implemented)
- [x] `daemon stop <name>` — Placeholder (not implemented)
- [x] `daemon restart <name>` — Placeholder (not implemented)
- [x] `daemon watchdog` — Placeholder (not implemented)

### Filesystem

- [x] `fs info` — FS Größe/Usage
- [x] `fs ls [path]` — Dateien auflisten
- [x] `fs cat <file>` — Dateiinhalt ausgeben
- [x] `fs rm <file>` — Datei löschen
- [x] `fs test` — FS-Testsuite ausführen

### NVS & Config

- [x] `nvs list` — bekannte NVS Keys anzeigen
- [x] `nvs get <key>` — NVS Wert lesen
- [x] `nvs set <key> <value>` — NVS Wert schreiben
- [x] `config reload` — `/config/config.yml` neu laden
- [x] `config show` — geladene Config anzeigen

### Unit Tests

- [x] `rut` — Required Unit Tests beim nächsten Boot erzwingen
- [x] `ut required` — Alias für `rut`
- [x] `ut optional` — optionale UT sofort ausführen
- [x] `ut fs` — FS-UT sofort ausführen
- [x] `ut rtos` — RTOS-UT sofort ausführen
- [x] `ut status` — UT-Status aus NVS

### Display & Peripherals

- [x] `display ping` — Display-Diagnose
- [x] `display test` — Alias für `display ping`
- [x] `display clear` — Display löschen
- [x] `display text <msg>` — Text anzeigen
- [x] `display brightness <0-100>` — Placeholder (not implemented)
- [x] `sound beep` — Placeholder (not implemented)
- [x] `sensor status` — Placeholder (not implemented)

## Hinweise

- [x] Nicht implementierte Commands sind bereits registriert und geben explizit `not implemented yet` zurück.
- [x] Damit bleibt die Command-Oberfläche stabil, während Handler schrittweise ergänzt werden.
