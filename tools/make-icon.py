#!/usr/bin/env python3
"""Create the 600px Bible launcher icon without third-party libraries."""

import struct
import zlib
from pathlib import Path

SIZE = 600
WHITE = (255, 255, 255, 255)
BLACK = (0, 0, 0, 255)

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


# A closed Bible, with a bound spine, visible pages, and a cover cross.
line(140, 95, 460, 95, 26, BLACK)
line(460, 95, 460, 435, 26, BLACK)
line(460, 435, 140, 435, 26, BLACK)
line(140, 435, 140, 95, 26, BLACK)
line(190, 95, 190, 435, 18, BLACK)
line(140, 435, 140, 495, 26, BLACK)
line(140, 495, 460, 495, 26, BLACK)
line(460, 495, 460, 435, 26, BLACK)
line(180, 465, 430, 465, 8, BLACK)
line(325, 180, 325, 350, 26, BLACK)
line(265, 240, 385, 240, 26, BLACK)


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
