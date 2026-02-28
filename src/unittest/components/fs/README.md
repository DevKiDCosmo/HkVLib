# Filesystem Test Plan (Unit + Integration-Nah)

Dieses Dokument beschreibt, **was im Filesystem geprüft werden sollte** und in welcher Reihenfolge.

## Entry Points

- Main Entry Point für FS-Checks: `UnitTest::runFsMainCheck()`
- Implementierung: `src/unittest/components/fs/fs.cpp`
- Deklaration: `src/unittest/components/fs/fs.h`
- Gemeinsame Hilfsfunktionen: `src/unittest/components/fs/fs_common.cpp` / `.h`
- Teil-Entry-Points:
	- `UnitTest::runFsMountCheck()`
	- `UnitTest::runFsFileCheck()`
	- `UnitTest::runFsDirectoryCheck()`
	- `UnitTest::runFsErrorCheck()`
	- `UnitTest::runFsPersistenceCheck()`
	- `UnitTest::runFsHandleCycleCheck()`
- Check-Dateien liegen unter: `src/unittest/components/fs/checks/`
- Legacy-Storage-Test (`runStorageTest`) wurde entfernt und in den FS-Checks integriert.

## Aktuell implementierte FS-Checks

- [x] Mount-Erkennung + On-Demand-Mount + Cleanup
- [x] Datei-Integrität (write/overwrite/append/read/rename/delete)
- [x] Verzeichnis-Checks (mkdir/readdir/nicht-leer-rmdir/rmdir)
- [x] Fehlerpfade (open/remove/opendir auf fehlende Ziele)
- [x] Persistenz (close/reopen und Payload-Konsistenz)
- [x] Handle-Zyklus (wiederholtes open/append/close inkl. Größenprüfung)
