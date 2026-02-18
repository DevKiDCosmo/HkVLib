#!/usr/bin/env python3
"""
ESP32 serial monitor with command input.

Serial output scrolls normally. The last line is the input prompt:
  > ...          (gray placeholder when empty)
  > my_command   (user typing)

Press Enter to send, Ctrl+C to exit.
"""

import argparse
import atexit
import os
import re
import select
import signal
import sys
import threading
import time

try:
    import serial
except ImportError as exc:
    raise SystemExit(
        "pyserial is required.  Install with:  pip install pyserial"
    ) from exc

# ---------------------------------------------------------------------------
# helpers
# ---------------------------------------------------------------------------

_ANSI_RE = re.compile(r"\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])")

# ANSI codes
CSI = "\033["
ERASE_LINE = f"{CSI}2K"  # erase entire current line
SHOW_CURSOR = f"{CSI}?25h"
HIDE_CURSOR = f"{CSI}?25l"
DIM = f"{CSI}2m"
RESET = f"{CSI}0m"
CYAN = f"{CSI}36m"
GREEN = f"{CSI}32m"
YELLOW = f"{CSI}33m"
RED = f"{CSI}31m"


def _terminal_width() -> int:
    try:
        return os.get_terminal_size().columns
    except OSError:
        return 80


# ---------------------------------------------------------------------------
# SerialMonitor
# ---------------------------------------------------------------------------


class SerialMonitor:
    def __init__(self, port: str, baud: int, strip_colors: bool = False):
        self.port = port
        self.baud = baud
        self.strip_colors = strip_colors

        self.ser = None  # type: serial.Serial | None
        self.running = True
        self.input_buf = ""

        # protects all sys.stdout usage
        self._lock = threading.Lock()

        # original terminal state (set in run())
        self._old_termios = None
        self._fd = None

    # ----- terminal helpers ------------------------------------------------

    def _setup_terminal(self):
        """Switch stdin to raw mode so we get chars one at a time."""
        import termios, tty

        self._fd = sys.stdin.fileno()
        self._old_termios = termios.tcgetattr(self._fd)
        tty.setraw(self._fd)

        # register restore so we *always* leave the terminal clean
        atexit.register(self._restore_terminal)

    def _restore_terminal(self):
        """Put the terminal back to its original state."""
        import termios

        if self._old_termios is not None and self._fd is not None:
            try:
                termios.tcsetattr(self._fd, termios.TCSADRAIN, self._old_termios)
            except Exception:
                pass
            self._old_termios = None
        # make sure cursor is visible
        sys.stdout.write(SHOW_CURSOR)
        sys.stdout.flush()

    # ----- output helpers (must hold self._lock) --------------------------

    def _clear_prompt(self):
        """Erase the prompt line so we can print a log line in its place."""
        sys.stdout.write(f"\r{ERASE_LINE}")

    def _draw_prompt(self):
        """Draw the prompt at the current cursor position."""
        if self.input_buf:
            sys.stdout.write(f"\r{ERASE_LINE}{GREEN}>{RESET} {self.input_buf}")
        else:
            sys.stdout.write(f"\r{ERASE_LINE}{GREEN}>{RESET}{DIM} ...{RESET}")
        sys.stdout.flush()

    def _print_line(self, text: str):
        """Print a log line above the prompt."""
        with self._lock:
            self._clear_prompt()
            sys.stdout.write(f"\r{text}\r\n")
            self._draw_prompt()

    def _status(self, text: str):
        """Print a status/info message."""
        self._print_line(f"{CYAN}[{text}]{RESET}")

    def _error(self, text: str):
        """Print an error message."""
        self._print_line(f"{RED}[{text}]{RESET}")

    # ----- serial I/O -----------------------------------------------------

    def _connect(self) -> bool:
        try:
            self.ser = serial.Serial(self.port, self.baud, timeout=0.1)
            self._status(f"Connected to {self.port} at {self.baud} baud")
            return True
        except serial.SerialException as exc:
            self._error(f"Cannot open {self.port}: {exc}")
            return False

    def _serial_reader(self):
        """Background thread — reads serial data and prints it."""
        buf = b""
        while self.running:
            # (re)connect
            while self.running and (self.ser is None or not self.ser.is_open):
                if self._connect():
                    break
                time.sleep(0.5)

            if not self.running:
                break

            try:
                chunk = self.ser.read(self.ser.in_waiting or 1)
                if not chunk:
                    continue

                buf += chunk

                # split on newlines and process complete lines
                while b"\n" in buf:
                    line_bytes, buf = buf.split(b"\n", 1)
                    line = line_bytes.decode("utf-8", errors="replace")
                    line = line.rstrip("\r")

                    if self.strip_colors:
                        line = _ANSI_RE.sub("", line)

                    self._print_line(line)

            except serial.SerialException as exc:
                self._error(f"Serial read error: {exc}")
                try:
                    self.ser.close()
                except Exception:
                    pass
                self.ser = None
                time.sleep(0.5)
            except Exception:
                time.sleep(0.05)

    def _send(self, cmd: str):
        """Send a command string to the serial port."""
        if self.ser and self.ser.is_open:
            try:
                self.ser.write((cmd + "\r\n").encode("utf-8"))
                self.ser.flush()
                self._print_line(f"{YELLOW}>>> {cmd}{RESET}")
            except serial.SerialException as exc:
                self._error(f"Send error: {exc}")
        else:
            self._error("Not connected")

    # ----- keyboard input (runs on main thread) ---------------------------

    def _handle_key(self, ch: str):
        if ch == "\r" or ch == "\n":
            cmd = self.input_buf
            self.input_buf = ""
            if cmd:
                self._send(cmd)
            else:
                # just redraw empty prompt
                with self._lock:
                    self._draw_prompt()
        elif ch == "\x7f" or ch == "\b":  # backspace
            self.input_buf = self.input_buf[:-1]
            with self._lock:
                self._draw_prompt()
        elif ch == "\x03":  # Ctrl+C
            self.running = False
        elif ch == "\x04":  # Ctrl+D
            self.running = False
        elif ch == "\x15":  # Ctrl+U  — clear line
            self.input_buf = ""
            with self._lock:
                self._draw_prompt()
        elif ch == "\x17":  # Ctrl+W  — delete word
            self.input_buf = self.input_buf.rstrip()
            i = self.input_buf.rfind(" ")
            self.input_buf = self.input_buf[: i + 1] if i >= 0 else ""
            with self._lock:
                self._draw_prompt()
        elif ord(ch) >= 32:  # printable
            self.input_buf += ch
            with self._lock:
                self._draw_prompt()

    # ----- main entry point -----------------------------------------------

    def run(self) -> int:
        self._setup_terminal()
        sys.stdout.write(HIDE_CURSOR)

        self._status(f"Serial monitor — {self.port} @ {self.baud}")
        self._status("Type a command and press Enter to send.  Ctrl+C to exit.")

        # draw initial prompt
        with self._lock:
            self._draw_prompt()

        # start serial reader in background
        t = threading.Thread(target=self._serial_reader, daemon=True)
        t.start()

        # read keyboard on the main thread (so signal handling works)
        try:
            while self.running:
                if select.select([sys.stdin], [], [], 0.1)[0]:
                    ch = sys.stdin.read(1)
                    if not ch:
                        break
                    self._handle_key(ch)
        except KeyboardInterrupt:
            pass
        finally:
            self.running = False
            self._restore_terminal()
            sys.stdout.write("\r\n")
            sys.stdout.flush()
            if self.ser and self.ser.is_open:
                self.ser.close()

        return 0


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------


def main() -> int:
    parser = argparse.ArgumentParser(
        description="ESP32 serial monitor with command input"
    )
    parser.add_argument(
        "--port",
        required=True,
        help="Serial port  (e.g. /dev/cu.usbserial-110)",
    )
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate")
    parser.add_argument(
        "--strip",
        action="store_true",
        help="Strip ANSI color codes from serial output",
    )
    args = parser.parse_args()

    mon = SerialMonitor(args.port, args.baud, args.strip)
    return mon.run()


if __name__ == "__main__":
    sys.exit(main())
