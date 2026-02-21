#!/usr/bin/env python3
"""
Debug script to analyze and test bitmap header files.
Reads RGB565 bitmap data from C++ headers and provides validation/visualization.
"""

import argparse
import re
import sys
from pathlib import Path
from collections import defaultdict
from typing import Optional, Tuple


class BitmapDebugger:
    def __init__(self, header_path: Path):
        self.path = header_path
        self.width = 0
        self.height = 0
        self.pixels = []
        self.symbol_prefix = ""
        self.parse()

    def parse(self):
        """Parse C++ bitmap header file."""
        content = self.path.read_text()

        # Extract symbol prefix and dimensions
        width_match = re.search(r'static constexpr uint8_t (\w+)_width = (\d+);', content)
        height_match = re.search(r'static constexpr uint8_t (\w+)_height = (\d+);', content)

        if not width_match or not height_match:
            raise ValueError("Could not find width/height in bitmap header")

        self.symbol_prefix = width_match.group(1)
        self.width = int(width_match.group(2))
        self.height = int(height_match.group(2))

        # Extract pixel data
        pixels_match = re.search(r'static const uint16_t \w+\[.*?\] = \{(.*?)\};', content, re.DOTALL)
        if not pixels_match:
            raise ValueError("Could not find pixel data in bitmap header")

        hex_str = pixels_match.group(1)
        hex_values = re.findall(r'0x([0-9A-Fa-f]+)', hex_str)
        self.pixels = [int(h, 16) for h in hex_values]

        if len(self.pixels) != self.width * self.height:
            raise ValueError(
                f"Pixel count mismatch: expected {self.width * self.height}, got {len(self.pixels)}"
            )

    def rgb565_to_rgb888(self, rgb565: int) -> Tuple[int, int, int]:
        """Convert RGB565 to RGB888."""
        r = (rgb565 >> 11) & 0x1F
        g = (rgb565 >> 5) & 0x3F
        b = rgb565 & 0x1F

        # Scale to 8-bit
        r = (r << 3) | (r >> 2)
        g = (g << 2) | (g >> 4)
        b = (b << 3) | (b >> 2)

        return r, g, b

    def analyze_colors(self):
        """Analyze color distribution."""
        color_count = defaultdict(int)
        for pixel in self.pixels:
            color_count[pixel] += 1

        sorted_colors = sorted(color_count.items(), key=lambda x: x[1], reverse=True)
        return sorted_colors[:10]  # Top 10 colors

    def detect_byte_swap(self) -> bool:
        """Detect if colors appear to be byte-swapped."""
        # Sample first non-black pixel and check if byte-swapped makes more sense
        for pixel in self.pixels:
            if pixel != 0x0000:
                swapped = ((pixel & 0xFF) << 8) | (pixel >> 8)
                
                # Extract RGB from both
                r_orig = (pixel >> 11) & 0x1F
                g_orig = (pixel >> 5) & 0x3F
                b_orig = pixel & 0x1F
                
                r_swap = (swapped >> 11) & 0x1F
                g_swap = (swapped >> 5) & 0x3F
                b_swap = swapped & 0x1F
                
                # If original looks like mostly low values and swapped has better distribution, likely swapped
                return (r_swap + g_swap + b_swap) > (r_orig + g_orig + b_orig)

        return False

    def generate_ppm(self, output_path: Path, swap: bool = False):
        """Generate PPM image file for visual inspection."""
        pixels_rgb = []
        for pixel in self.pixels:
            if swap:
                pixel = ((pixel & 0xFF) << 8) | (pixel >> 8)
            r, g, b = self.rgb565_to_rgb888(pixel)
            pixels_rgb.extend([r, g, b])

        with open(output_path, 'wb') as f:
            # PPM header
            f.write(f"P6\n{self.width} {self.height}\n255\n".encode())
            # Pixel data
            for rgb in pixels_rgb:
                f.write(bytes([rgb]))

    def validate(self) -> dict:
        """Run validation checks."""
        issues = []
        warnings = []

        # Check dimensions
        if self.width != self.height:
            warnings.append(f"Non-square bitmap: {self.width}x{self.height}")

        if self.width > 256 or self.height > 256:
            issues.append("Dimensions exceed 256 (uint8_t limit)")

        # Check pixel count
        if len(self.pixels) != self.width * self.height:
            issues.append(f"Pixel count mismatch: expected {self.width * self.height}, got {len(self.pixels)}")

        # Check for mostly black (corrupt/empty bitmap)
        black_count = sum(1 for p in self.pixels if p == 0x0000)
        black_ratio = black_count / len(self.pixels)
        if black_ratio > 0.95:
            warnings.append(f"Bitmap is {black_ratio*100:.1f}% black - might be corrupt or blank")

        # Check for likely byte-swap issues
        if self.detect_byte_swap():
            warnings.append("Colors appear to be byte-swapped (little-endian issue)")

        return {
            "valid": len(issues) == 0,
            "issues": issues,
            "warnings": warnings,
            "dimensions": (self.width, self.height),
            "pixel_count": len(self.pixels),
            "unique_colors": len(set(self.pixels))
        }

    def print_info(self):
        """Print detailed bitmap information."""
        print(f"\n{'='*60}")
        print(f"Bitmap Debug: {self.path.name}")
        print(f"{'='*60}")
        print(f"Symbol prefix: {self.symbol_prefix}")
        print(f"Dimensions: {self.width}x{self.height}")
        print(f"Pixel count: {len(self.pixels)}")
        print(f"Unique colors: {len(set(self.pixels))}")

        # Color analysis
        print(f"\nTop 10 most common colors:")
        for i, (color, count) in enumerate(self.analyze_colors(), 1):
            r, g, b = self.rgb565_to_rgb888(color)
            percentage = (count / len(self.pixels)) * 100
            print(f"  {i}. 0x{color:04X} (RGB: {r:3d}, {g:3d}, {b:3d}) - {count:5d} pixels ({percentage:5.2f}%)")

        # Byte swap check
        if self.detect_byte_swap():
            print(f"\n⚠️  WARNING: Bitmap appears byte-swapped!")
            print(f"   Consider regenerating with byte-swap enabled.")

        # Validation
        print(f"\n{'─'*60}")
        validation = self.validate()
        print(f"Validation: {'✓ PASS' if validation['valid'] else '✗ FAIL'}")

        if validation['issues']:
            print(f"\nIssues ({len(validation['issues'])}):")
            for issue in validation['issues']:
                print(f"  ✗ {issue}")

        if validation['warnings']:
            print(f"\nWarnings ({len(validation['warnings'])}):")
            for warning in validation['warnings']:
                print(f"  ⚠️  {warning}")

        print(f"{'='*60}\n")

    def compare_with(self, other: 'BitmapDebugger') -> dict:
        """Compare two bitmaps."""
        if self.width != other.width or self.height != other.height:
            return {
                "compatible": False,
                "reason": f"Dimension mismatch: {self.width}x{self.height} vs {other.width}x{other.height}"
            }

        if len(self.pixels) != len(other.pixels):
            return {
                "compatible": False,
                "reason": f"Pixel count mismatch: {len(self.pixels)} vs {len(other.pixels)}"
            }

        # Check if byte-swapped version of each other
        is_byte_swapped = all(
            ((a & 0xFF) << 8) | (a >> 8) == b
            for a, b in zip(self.pixels, other.pixels)
        )

        if is_byte_swapped:
            return {
                "compatible": True,
                "relationship": "One is byte-swapped version of the other",
                "identical_pixels": 0,
                "byte_swapped_match": len(self.pixels)
            }

        # Count differences
        differences = sum(1 for a, b in zip(self.pixels, other.pixels) if a != b)
        identical = len(self.pixels) - differences

        return {
            "compatible": True,
            "identical_pixels": identical,
            "different_pixels": differences,
            "match_ratio": (identical / len(self.pixels)) * 100
        }


def main():
    parser = argparse.ArgumentParser(description="Debug bitmap header files")
    parser.add_argument("header", type=Path, help="Bitmap header file to debug")
    parser.add_argument("--compare", type=Path, help="Compare with another bitmap header")
    parser.add_argument("--export-ppm", type=Path, help="Export as PPM image file")
    parser.add_argument("--swap", action="store_true", help="Apply byte-swap when exporting PPM")
    parser.add_argument("--silent", action="store_true", help="Suppress output")

    args = parser.parse_args()

    try:
        bitmap = BitmapDebugger(args.header)

        if not args.silent:
            bitmap.print_info()

        if args.compare:
            bitmap2 = BitmapDebugger(args.compare)
            comparison = bitmap.compare_with(bitmap2)
            print(f"Comparison: {args.header.name} vs {args.compare.name}")
            print(f"{'─'*60}")
            for key, value in comparison.items():
                print(f"  {key}: {value}")
            print()

        if args.export_ppm:
            bitmap.generate_ppm(args.export_ppm, swap=args.swap)
            print(f"✓ Exported PPM image to {args.export_ppm}")

        return 0

    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
