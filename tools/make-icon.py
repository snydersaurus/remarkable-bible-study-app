#!/usr/bin/env python3
"""Create the neutral 600px launcher icon without third-party libraries."""

import struct
import zlib
from pathlib import Path

SIZE = 600
WHITE = (255, 255, 255, 255)
BLACK = (0, 0, 0, 255)
ACCENT = (164, 18, 63, 255)

pixels = [[WHITE for _ in range(SIZE)] for _ in range(SIZE)]


def set_pixel(x, y, color):
    if 0 <= x < SIZE and 0 <= y < SIZE:
        pixels[y][x] = color


def line(x0, y0, x1, y1, width, color):
    steps = max(abs(x1 - x0), abs(y1 - y0))
    radius = max(1, width // 2)
    for step in range(steps + 1):
        t = step / steps if steps else 0
        x = round(x0 + (x1 - x0) * t)
        y = round(y0 + (y1 - y0) * t)
        for dy in range(-radius, radius + 1):
            for dx in range(-radius, radius + 1):
                if dx * dx + dy * dy <= radius * radius:
                    set_pixel(x + dx, y + dy, color)


# Open-book mark with a restrained accent rule.
line(110, 150, 300, 205, 26, BLACK)
line(300, 205, 490, 150, 26, BLACK)
line(110, 150, 110, 420, 26, BLACK)
line(110, 420, 300, 475, 26, BLACK)
line(300, 475, 490, 420, 26, BLACK)
line(490, 420, 490, 150, 26, BLACK)
line(300, 205, 300, 475, 24, BLACK)
line(155, 525, 445, 525, 18, ACCENT)


def chunk(kind, data):
    return (struct.pack(">I", len(data)) + kind + data
            + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF))


raw = b"".join(
    b"\x00" + bytes(channel for pixel in row for channel in pixel)
    for row in pixels
)
png = (b"\x89PNG\r\n\x1a\n"
       + chunk(b"IHDR", struct.pack(">IIBBBBB", SIZE, SIZE, 8, 6, 0, 0, 0))
       + chunk(b"IDAT", zlib.compress(raw, 9))
       + chunk(b"IEND", b""))

output = Path(__file__).resolve().parent.parent / "appload-native" / "icon.png"
output.write_bytes(png)
print(output)
