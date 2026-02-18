# DHCP ID Server
#
# Assigns unique device IDs to ESP32 robots based on their MAC address and team.
# Persists the device-to-team database periodically to a CSV file.

import csv
import os
import threading
import time
from datetime import datetime
from typing import Optional

from flask import Blueprint, jsonify

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

CSV_PATH = os.getenv(
    "DHCPID_CSV_PATH",
    os.path.join(os.path.dirname(__file__), "..", "data", "devices.csv"),
)
SAVE_INTERVAL = int(os.getenv("DHCPID_SAVE_INTERVAL", "30"))  # seconds

# ---------------------------------------------------------------------------
# In-memory device database
# ---------------------------------------------------------------------------

# { mac_address: { "device_id": int, "team_id": int, "first_seen": str, "last_seen": str } }
_devices: dict[str, dict] = {}
_lock = threading.Lock()
_next_id: int = 1  # auto-incrementing device ID counter
_dirty: bool = False  # tracks whether in-memory state has unsaved changes


def _now_iso() -> str:
    return datetime.utcnow().isoformat(timespec="seconds") + "Z"


def _assign_id(mac: str, team_id: int) -> int:
    """Return the device ID for *mac*, creating a new entry if necessary."""
    global _next_id, _dirty

    with _lock:
        entry = _devices.get(mac)
        if entry is not None:
            # Update team_id in case the device was re-assigned
            entry["team_id"] = team_id
            entry["last_seen"] = _now_iso()
            _dirty = True
            return entry["device_id"]

        # New device — assign next available ID
        device_id = _next_id
        _next_id += 1
        _devices[mac] = {
            "device_id": device_id,
            "team_id": team_id,
            "first_seen": _now_iso(),
            "last_seen": _now_iso(),
        }
        _dirty = True
        return device_id


def _remove_device(mac: str) -> bool:
    """Remove a device from the registry. Returns True if found."""
    global _dirty

    with _lock:
        if mac in _devices:
            del _devices[mac]
            _dirty = True
            return True
        return False


def _get_all_devices() -> list[dict]:
    """Return a snapshot of all registered devices."""
    with _lock:
        result = []
        for mac, info in _devices.items():
            result.append({"mac": mac, **info})
        return result


# ---------------------------------------------------------------------------
# CSV persistence
# ---------------------------------------------------------------------------

_CSV_FIELDS = ["mac", "device_id", "team_id", "first_seen", "last_seen"]


def _ensure_data_dir() -> None:
    data_dir = os.path.dirname(CSV_PATH)
    if data_dir and not os.path.exists(data_dir):
        os.makedirs(data_dir, exist_ok=True)


def load_csv() -> None:
    """Load devices from the CSV file into memory (called once at startup)."""
    global _next_id

    if not os.path.exists(CSV_PATH):
        return

    with _lock:
        with open(CSV_PATH, newline="") as f:
            reader = csv.DictReader(f)
            for row in reader:
                mac = row["mac"]
                device_id = int(row["device_id"])
                team_id = int(row["team_id"])
                _devices[mac] = {
                    "device_id": device_id,
                    "team_id": team_id,
                    "first_seen": row.get("first_seen", ""),
                    "last_seen": row.get("last_seen", ""),
                }
                # Keep _next_id above the highest loaded ID
                if device_id >= _next_id:
                    _next_id = device_id + 1


def save_csv() -> None:
    """Write the current device database to CSV."""
    global _dirty

    with _lock:
        if not _dirty:
            return
        _ensure_data_dir()
        tmp_path = CSV_PATH + ".tmp"
        with open(tmp_path, "w", newline="") as f:
            writer = csv.DictWriter(f, fieldnames=_CSV_FIELDS)
            writer.writeheader()
            for mac, info in _devices.items():
                writer.writerow({"mac": mac, **info})
        os.replace(tmp_path, CSV_PATH)
        _dirty = False


# ---------------------------------------------------------------------------
# Background save thread
# ---------------------------------------------------------------------------

_save_thread: Optional[threading.Thread] = None
_stop_event = threading.Event()


def _periodic_save() -> None:
    """Runs in a daemon thread; saves the CSV every SAVE_INTERVAL seconds."""
    while not _stop_event.is_set():
        _stop_event.wait(SAVE_INTERVAL)
        if _stop_event.is_set():
            break
        try:
            save_csv()
        except Exception as exc:
            print(f"[dhcpid] CSV save error: {exc}")


def start_save_thread() -> None:
    """Start the periodic CSV save background thread."""
    global _save_thread
    if _save_thread is not None:
        return
    _save_thread = threading.Thread(
        target=_periodic_save, daemon=True, name="dhcpid-csv-save"
    )
    _save_thread.start()


def stop_save_thread() -> None:
    """Signal the save thread to stop and do a final save."""
    _stop_event.set()
    if _save_thread is not None:
        _save_thread.join(timeout=5)
    try:
        save_csv()
    except Exception as exc:
        print(f"[dhcpid] Final CSV save error: {exc}")


# ---------------------------------------------------------------------------
# Flask Blueprint
# ---------------------------------------------------------------------------

dhcpid_bp = Blueprint("dhcpid", __name__)


@dhcpid_bp.get("/id/<mac>/<int:teamid>")
def get_or_assign_id(mac: str, teamid: int):
    """
    GET /id/<mac>/<teamid>
    Returns the numeric device ID for the given MAC + team.
    Creates a new entry if the MAC hasn't been seen before.
    The firmware expects a plain-text integer response.
    """
    device_id = _assign_id(mac, teamid)
    # Firmware parses the body with .toInt(), so return plain text
    return str(device_id), 200


@dhcpid_bp.get("/devices")
def list_devices():
    """
    GET /devices
    Returns JSON array of all registered devices.
    """
    return jsonify(_get_all_devices()), 200


@dhcpid_bp.get("/devices/<mac>")
def get_device(mac: str):
    """
    GET /devices/<mac>
    Returns JSON for a single device or 404.
    """
    with _lock:
        entry = _devices.get(mac)
    if entry is None:
        return jsonify({"error": "device not found"}), 404
    return jsonify({"mac": mac, **entry}), 200


@dhcpid_bp.delete("/devices/<mac>")
def delete_device(mac: str):
    """
    DELETE /devices/<mac>
    Removes a device from the registry.
    """
    if _remove_device(mac):
        return jsonify({"status": "deleted", "mac": mac}), 200
    return jsonify({"error": "device not found"}), 404


@dhcpid_bp.get("/devices/save")
def force_save():
    """
    GET /devices/save
    Force an immediate CSV save.
    """
    try:
        save_csv()
        return jsonify({"status": "saved", "path": CSV_PATH}), 200
    except Exception as exc:
        return jsonify({"error": str(exc)}), 500
