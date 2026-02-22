# Distributed PlatformIO Build Setup (macOS Cluster)
## distcc + ccache mit mehreren Macs

Diese Anleitung beschreibt die vollständige Einrichtung eines macOS Build-Clusters mit:

- distcc (verteiltes Kompilieren)
- ccache (Compiler Cache)
- PlatformIO
- mehreren Macs im gleichen Netzwerk

Ziel: Buildzeiten von ~5 Minuten auf 1–2 Minuten reduzieren.

---

# 1. Voraussetzungen

- Alle Macs im gleichen LAN (z. B. 192.168.0.x)
- Gleiche CPU-Architektur (alle Intel oder alle Apple Silicon)
- PlatformIO installiert
- Adminrechte

---

# 2. Installation (auf ALLEN Macs)

## 2.1 Homebrew installieren (falls nicht vorhanden)

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Shell neu starten.

Prüfen:

```bash
brew --version
```

---

## 2.2 distcc und ccache installieren

```bash
brew update
brew install distcc ccache
```

Prüfen:

```bash
distcc --version
ccache --version
```

---

# 3. Toolchain vorbereiten

WICHTIG: Alle Macs müssen identische Toolchains besitzen.

Einfachste Methode:

Auf jedem Mac einmal im gleichen Projekt ausführen:

```bash
pio run
```

PlatformIO installiert automatisch:

```
~/.platformio/packages/
```

Beispiel für ESP32:

```
~/.platformio/packages/toolchain-xtensa-esp32/
```

---

# 4. Worker-Macs konfigurieren

Auf jedem Worker-Mac:

## 4.1 distccd starten (Testmodus)

```bash
distccd \
  --no-detach \
  --listen 0.0.0.0 \
  --allow 192.168.0.0/24 \
  --jobs 8 \
  --enable-tcp-insecure \
  --verbose
```

Erklärung:

* `--listen 0.0.0.0` → akzeptiert Netzwerkverbindungen
* `--allow 192.168.0.0/24` → erlaubt dein LAN
* `--jobs 8` → Anzahl CPU-Kerne
* `--enable-tcp-insecure` → nötig bei Homebrew-Installation

Core-Anzahl prüfen:

```bash
sysctl -n hw.ncpu
```

---

## 4.2 Firewall prüfen

System Settings → Network → Firewall
distccd erlauben oder Firewall deaktivieren.

---

## 4.3 Dauerbetrieb (Daemon)

Wenn alles funktioniert:

```bash
distccd \
  --daemon \
  --listen 0.0.0.0 \
  --allow 192.168.0.0/24 \
  --jobs 8 \
  --enable-tcp-insecure \
  --log-file ~/distccd.log
```

---

# 5. Master-Mac konfigurieren

## 5.1 Hosts-Datei erstellen

```bash
mkdir -p ~/.distcc
nano ~/.distcc/hosts
```

Beispiel:

```
192.168.0.171/8,lzo
192.168.0.172/8,lzo
localhost/8
```

Test:

```bash
distcc --show-hosts
```

---

## 5.2 ccache konfigurieren

```bash
ccache --max-size=50G
ccache --set-config=compression=true
```

Optional (empfohlen):

```bash
export CCACHE_BASEDIR=$(pwd)
export CCACHE_SLOPPINESS=time_macros
```

In `~/.zshrc` eintragen.

---

# 6. Compiler-Wrapper für PlatformIO

Nur auf dem Master-Mac.

## 6.1 Wrapper-Ordner erstellen

```bash
mkdir -p ~/pio-wrappers
cd ~/pio-wrappers
```

---

## 6.2 GCC Wrapper (ESP32 Beispiel)

Datei erstellen:

```bash
nano xtensa-esp32-elf-gcc
```

Inhalt:

```bash
#!/bin/bash
exec ccache distcc ~/.platformio/packages/toolchain-xtensa-esp32/bin/xtensa-esp32-elf-gcc "$@"
```

Speichern.

---

## 6.3 G++ Wrapper

```bash
nano xtensa-esp32-elf-g++
```

Inhalt:

```bash
#!/bin/bash
exec ccache distcc ~/.platformio/packages/toolchain-xtensa-esp32/bin/xtensa-esp32-elf-g++ "$@"
```

---

## 6.4 Executable setzen

```bash
chmod +x xtensa-esp32-elf-gcc
chmod +x xtensa-esp32-elf-g++
```

---

## 6.5 PATH anpassen

```bash
echo 'export PATH=~/pio-wrappers:$PATH' >> ~/.zshrc
source ~/.zshrc
```

---

# 7. Build starten

Empfohlene Parallelisierung:

Gesamt-Jobs = Summe aller CPU-Kerne

Beispiel:
2 Macs × 8 Kerne = 16

```bash
export DISTCC_HOSTS="192.168.0.171/8,lzo 192.168.0.172/8,lzo localhost/8"
pump pio run -j 20
```

---

# 8. Monitoring

Live-Überwachung:

```bash
distccmon-text 1
```

Cache-Statistik:

```bash
ccache -s
```

Worker-Log:

```bash
tail -f ~/distccd.log
```

---

# 9. Fehlerdiagnose

## distccd startet nicht

Im Debug-Modus starten:

```bash
distccd --no-detach --verbose ...
```

---

## Worker wird nicht genutzt

Prüfen:

```bash
nc 192.168.0.171 3632
```

Firewall prüfen.

---

## Nur ein Mac rechnet

Meist:

* Toolchain nicht identisch
* Architektur-Mismatch (Intel vs ARM)
* falscher Compiler-Pfad

---

# 10. Performance-Erwartung

| Setup  | Full Build |
| ------ | ---------- |
| 1 Mac  | ~5 min     |
| 2 Macs | ~2.5–3 min |
| 3 Macs | ~1.8 min   |
| 4 Macs | ~1.3 min   |

Mit ccache:
Rebuilds oft unter 30 Sekunden.

---

# 11. Optional: Optimierungen

* Gigabit oder 2.5G LAN
* Ethernet statt WLAN
* ccache auf schneller SSD
* pump mode dauerhaft nutzen

---

# 12. Architektur prüfen

Auf allen Macs:

```bash
uname -m
```

Alle müssen gleich sein:

* arm64
  oder
* x86_64

Mischen vermeiden.

---

# Info

Beucase PIO doesn't add the compiler directly, please add them in .zshrc.

```bash
echo 'export PATH=$HOME/.platformio/packages/toolchain-xtensa-esp32/bin:$PATH' >> ~/.zshrc
source ~/.zshrc
```