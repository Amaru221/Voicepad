#!/usr/bin/env python3
"""Generates Assets/icon.png for VoicePad: an amber CRT-console style app icon.

Pure standard library (no Pillow). Renders at 4x then box-downsamples for
anti-aliasing. Run from the repo root:  python Tools/make_icon.py
"""

import os
import struct
import zlib

OUT_SIZE = 256
SS = 4
N = OUT_SIZE * SS

# Palette (0..255 RGB)
BG_TOP = (22, 16, 9)
BG_BOT = (7, 5, 2)
AMBER = (255, 176, 0)
AMBER_BRIGHT = (255, 214, 120)
DARK = (12, 8, 3)


def lerp(a, b, t):
    return a + (b - a) * t


def mix(c0, c1, t):
    return (lerp(c0[0], c1[0], t), lerp(c0[1], c1[1], t), lerp(c0[2], c1[2], t))


def in_rounded(x, y, x0, y0, x1, y1, r):
    if x < x0 or x > x1 or y < y0 or y > y1:
        return False
    cx = min(max(x, x0 + r), x1 - r)
    cy = min(max(y, y0 + r), y1 - r)
    dx = x - cx
    dy = y - cy
    return dx * dx + dy * dy <= r * r


def pixel(x, y):
    # geometry (relative to N)
    m = 0.020 * N
    rad = 0.205 * N
    border = 0.030 * N
    x0, y0, x1, y1 = m, m, N - m, N - m

    if not in_rounded(x, y, x0, y0, x1, y1, rad):
        return (0, 0, 0, 0)

    # border band?
    if not in_rounded(x, y, x0 + border, y0 + border, x1 - border, y1 - border, max(1.0, rad - border)):
        # subtle top-to-bottom brightening of the frame
        t = y / N
        c = mix(AMBER_BRIGHT, AMBER, t)
        return (int(c[0]), int(c[1]), int(c[2]), 255)

    # background gradient
    bg = mix(BG_TOP, BG_BOT, y / N)

    cx = 0.5 * N
    hw = 0.105 * N
    cap_top = 0.235 * N
    cap_bot = 0.585 * N
    cap_r = hw

    # microphone capsule
    if in_rounded(x, y, cx - hw, cap_top, cx + hw, cap_bot, cap_r):
        # grille lines
        for gy in (0.310 * N, 0.370 * N, 0.430 * N):
            if abs(y - gy) < 0.014 * N and (cx - hw + 0.028 * N) < x < (cx + hw - 0.028 * N):
                return (DARK[0], DARK[1], DARK[2], 255)
        t = (y - cap_top) / max(1.0, (cap_bot - cap_top))
        c = mix(AMBER_BRIGHT, AMBER, t)
        return (int(c[0]), int(c[1]), int(c[2]), 255)

    # cradle (U arc under the capsule)
    cr_x, cr_y = cx, 0.565 * N
    R = 0.185 * N
    band = 0.050 * N
    if y >= cr_y:
        d = ((x - cr_x) ** 2 + (y - cr_y) ** 2) ** 0.5
        if abs(d - R) <= band * 0.5:
            return (int(AMBER[0]), int(AMBER[1]), int(AMBER[2]), 255)

    # stem
    if abs(x - cx) <= 0.023 * N and 0.735 * N <= y <= 0.815 * N:
        return (int(AMBER[0]), int(AMBER[1]), int(AMBER[2]), 255)

    # base
    bx = 0.135 * N
    if in_rounded(x, y, cx - bx, 0.800 * N, cx + bx, 0.865 * N, 0.030 * N):
        return (int(AMBER[0]), int(AMBER[1]), int(AMBER[2]), 255)

    return (int(bg[0]), int(bg[1]), int(bg[2]), 255)


def write_png(path, width, height, rows):
    raw = bytearray()
    for row in rows:
        raw.append(0)
        raw.extend(row)

    comp = zlib.compress(bytes(raw), 9)

    def chunk(typ, data):
        return (struct.pack(">I", len(data)) + typ + data
                + struct.pack(">I", zlib.crc32(typ + data) & 0xFFFFFFFF))

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n")
        f.write(chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)))
        f.write(chunk(b"IDAT", comp))
        f.write(chunk(b"IEND", b""))


def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    out_dir = os.path.join(root, "Assets")
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, "icon.png")

    # render supersampled
    hi = [[pixel(x, y) for x in range(N)] for y in range(N)]

    # box downsample
    rows = []
    inv = 1.0 / (SS * SS)
    for oy in range(OUT_SIZE):
        row = bytearray()
        for ox in range(OUT_SIZE):
            r = g = b = a = 0
            for sy in range(SS):
                line = hi[oy * SS + sy]
                base = ox * SS
                for sx in range(SS):
                    pr, pg, pb, pa = line[base + sx]
                    r += pr
                    g += pg
                    b += pb
                    a += pa
            row += bytes((int(r * inv + 0.5), int(g * inv + 0.5), int(b * inv + 0.5), int(a * inv + 0.5)))
        rows.append(row)

    write_png(out_path, OUT_SIZE, OUT_SIZE, rows)
    print("wrote", out_path, OUT_SIZE, "x", OUT_SIZE)


if __name__ == "__main__":
    main()
