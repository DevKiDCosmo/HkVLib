#!/usr/bin/env python3
from pathlib import Path
import argparse


def is_comment_or_empty(line: str) -> bool:
    stripped = line.strip()
    return not stripped or stripped.startswith("//")


def count_loc_in_file(file_path: Path) -> int:
    loc = 0
    in_block_comment = False

    for raw_line in file_path.read_text(encoding="utf-8", errors="ignore").splitlines():
        line = raw_line

        if in_block_comment:
            end = line.find("*/")
            if end == -1:
                continue
            line = line[end + 2 :]
            in_block_comment = False

        while True:
            start = line.find("/*")
            if start == -1:
                break
            end = line.find("*/", start + 2)
            if end == -1:
                line = line[:start]
                in_block_comment = True
                break
            line = line[:start] + line[end + 2 :]

        if not is_comment_or_empty(line):
            loc += 1

    return loc


def main() -> None:
    parser = argparse.ArgumentParser(description="Count LOC in source files.")
    parser.add_argument("path", nargs="?", default="src", help="Path to scan (default: src)")
    parser.add_argument(
        "--ext",
        nargs="+",
        default=[".c", ".cc", ".cpp", ".h", ".hh", ".hpp"],
        help="File extensions to include",
    )
    parser.add_argument("--details", action="store_true", help="Show per-file LOC")
    args = parser.parse_args()

    root = Path(args.path)
    if not root.exists() or not root.is_dir():
        raise SystemExit(f"Invalid directory: {root}")

    extensions = {e if e.startswith(".") else f".{e}" for e in args.ext}

    files = sorted(
        p for p in root.rglob("*")
        if p.is_file() and p.suffix.lower() in extensions
    )

    total = 0
    for file_path in files:
        loc = count_loc_in_file(file_path)
        total += loc
        if args.details:
            print(f"{file_path}: {loc}")

    print(f"Files scanned: {len(files)}")
    print(f"Total LOC: {total}")


if __name__ == "__main__":
    main()
