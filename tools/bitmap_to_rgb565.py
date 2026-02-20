#!/usr/bin/env python3
from __future__ import annotations

import argparse
import os
import struct
from pathlib import Path


def read_bmp_rgb(path: Path):
    with path.open("rb") as handle:
        data = handle.read()

    if len(data) < 54 or data[0:2] != b"BM":
        raise ValueError("Not a BMP file")

    pixel_offset = struct.unpack_from("<I", data, 10)[0]
    dib_size = struct.unpack_from("<I", data, 14)[0]
    if dib_size < 40:
        raise ValueError("Unsupported BMP DIB header")

    width = struct.unpack_from("<i", data, 18)[0]
    height_signed = struct.unpack_from("<i", data, 22)[0]
    planes = struct.unpack_from("<H", data, 26)[0]
    bpp = struct.unpack_from("<H", data, 28)[0]
    compression = struct.unpack_from("<I", data, 30)[0]

    if planes != 1:
        raise ValueError("Invalid BMP planes")
    if compression != 0:
        raise ValueError("Compressed BMP is not supported")
    if bpp not in (8, 24, 32):
        raise ValueError("Only 8-bit, 24-bit and 32-bit BMP files are supported")

    width_abs = abs(width)
    height_abs = abs(height_signed)
    top_down = height_signed < 0

    bytes_per_pixel = bpp // 8
    row_stride = ((width_abs * bytes_per_pixel + 3) // 4) * 4

    palette = None
    if bpp == 8:
        palette = []
        palette_start = 14 + dib_size
        palette_len = pixel_offset - palette_start
        if palette_len <= 0 or (palette_len % 4) != 0:
            raise ValueError("Invalid BMP palette")
        for idx in range(0, palette_len, 4):
            b = data[palette_start + idx]
            g = data[palette_start + idx + 1]
            r = data[palette_start + idx + 2]
            palette.append((r, g, b))

    pixels = []
    for row in range(height_abs):
        src_row = row if top_down else (height_abs - 1 - row)
        row_start = pixel_offset + src_row * row_stride
        row_pixels = []
        for col in range(width_abs):
            px = row_start + col * bytes_per_pixel
            if bpp == 8:
                color_idx = data[px]
                if color_idx >= len(palette):
                    raise ValueError("Palette index out of range")
                r, g, b = palette[color_idx]
            else:
                b = data[px]
                g = data[px + 1]
                r = data[px + 2]
            rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            row_pixels.append(rgb565)
        pixels.extend(row_pixels)

    return width_abs, height_abs, pixels


def write_header(out_path: Path, width: int, height: int, pixels: list[int], symbol_prefix: str):
    guard = f"{symbol_prefix.upper()}_H"
    array_name = f"{symbol_prefix}_pixels"
    width_name = f"{symbol_prefix}_width"
    height_name = f"{symbol_prefix}_height"

    lines = [
        "#pragma once",
        "",
        "#include <stdint.h>",
        "",
        f"static constexpr uint8_t {width_name} = {width};",
        f"static constexpr uint8_t {height_name} = {height};",
        f"static const uint16_t {array_name}[{width * height}] = {{",
    ]

    chunk = 12
    for index in range(0, len(pixels), chunk):
        part = pixels[index:index + chunk]
        encoded = ", ".join(f"0x{value:04X}" for value in part)
        lines.append(f"    {encoded},")

    lines.extend([
        "};",
        "",
    ])

    out_path.write_text("\n".join(lines), encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description="Convert BMP image to RGB565 C++ header")
    parser.add_argument("input", type=Path, help="Input BMP path")
    parser.add_argument("output", type=Path, help="Output .h path")
    parser.add_argument("--symbol", default="boot_bmp", help="Symbol prefix in generated header")
    args = parser.parse_args()

    width, height, pixels = read_bmp_rgb(args.input)
    if width > 255 or height > 255:
        raise ValueError("Bitmap dimensions must fit uint8_t")

    os.makedirs(args.output.parent, exist_ok=True)
    write_header(args.output, width, height, pixels, args.symbol)


if __name__ == "__main__":
    main()
