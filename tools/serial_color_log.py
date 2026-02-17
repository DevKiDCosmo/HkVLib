import argparse
import re
import sys
from time import sleep
try:
    import serial
except ImportError as exc:
    raise SystemExit(
        "pyserial is required. Install with: pip install pyserial"
    ) from exc


ANSI_COLOR_RE = re.compile(r"\x1b\[[0-9;]*m")


def main() -> int:
    parser = argparse.ArgumentParser(description="ESP32 serial monitor with ANSI colors")
    parser.add_argument("--port", required=True, help="Serial port, e.g. COM5")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate")
    parser.add_argument("--strip", action="store_true", help="Strip ANSI color codes")
    args = parser.parse_args()

    while True:
        try:
            with serial.Serial(args.port, args.baud, timeout=0.1) as ser:
                while True:
                    data = ser.readline()
                    if not data:
                        continue
                    text = data.decode("utf-8", errors="replace")
                    if args.strip:
                        text = ANSI_COLOR_RE.sub("", text)
                    sys.stdout.write(text)
                    sys.stdout.flush()
        except KeyboardInterrupt:
            return 0
        except serial.SerialException as exc:
            print(f"Serial error: {exc}", file=sys.stderr)
            # return 1
        sleep(1)  # Wait before retrying


if __name__ == "__main__":
    raise SystemExit(main())
