#!/usr/bin/env python3
"""Generate sci-fi city PPM textures for the raycasting engine."""

import math
import os
import random

TEX = 64
OUT = os.path.join(os.path.dirname(__file__), "..", "assets", "textures")


def cel(v, steps=6):
    step = 256 // steps
    return max(0, min(255, (v // step) * step + step // 2))


def write_ppm(path, pixels):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as f:
        f.write(f"P6\n{TEX} {TEX}\n255\n".encode())
        for y in range(TEX):
            for x in range(TEX):
                r, g, b = pixels[y * TEX + x]
                f.write(bytes([r, g, b]))


def neon_grid():
    px = [0] * (TEX * TEX)
    for y in range(TEX):
        for x in range(TEX):
            grid = (x % 8 == 0 or y % 8 == 0)
            glow = int(40 + 20 * math.sin(x * 0.5) * math.sin(y * 0.5))
            if grid:
                r, g, b = cel(0), cel(255), cel(220)
            else:
                r, g, b = cel(20), cel(10), cel(60 + glow)
            px[y * TEX + x] = (r, g, b)
    return px


def metal_panel():
    px = [0] * (TEX * TEX)
    for y in range(TEX):
        for x in range(TEX):
            rivet = (x % 16 == 4 and y % 16 == 4)
            scratch = (x + y * 3) % 17 == 0
            base = 90 + (x % 4) * 3
            r = cel(base + 10)
            g = cel(base + 5)
            b = cel(base + 20)
            if rivet:
                r, g, b = cel(180), cel(180), cel(200)
            if scratch:
                r, g, b = cel(220), cel(100), cel(50)
            px[y * TEX + x] = (r, g, b)
    return px


def city_lights():
    px = [0] * (TEX * TEX)
    random.seed(42)
    windows = {(random.randint(0, 63), random.randint(0, 63)) for _ in range(80)}
    for y in range(TEX):
        for x in range(TEX):
            if (x, y) in windows:
                hue = random.choice([(255, 80, 180), (80, 255, 220), (255, 200, 60)])
                px[y * TEX + x] = tuple(cel(c) for c in hue)
            else:
                px[y * TEX + x] = (cel(15), cel(18), cel(35))
    return px


def circuit():
    px = [0] * (TEX * TEX)
    for y in range(TEX):
        for x in range(TEX):
            line = x % 12 in (0, 1) or y % 12 in (0, 1)
            node = (x % 12 == 0 and y % 12 == 0)
            if node:
                px[y * TEX + x] = (cel(255), cel(50), cel(255))
            elif line:
                px[y * TEX + x] = (cel(0), cel(200), cel(255))
            else:
                px[y * TEX + x] = (cel(10), cel(30), cel(40))
    return px


def holo_wall():
    px = [0] * (TEX * TEX)
    for y in range(TEX):
        for x in range(TEX):
            band = int(127 + 127 * math.sin((x + y) * 0.25))
            r = cel(band)
            g = cel(255 - band // 2)
            b = cel(200)
            px[y * TEX + x] = (r, g, b)
    return px


def rust_pipe():
    px = [0] * (TEX * TEX)
    for y in range(TEX):
        for x in range(TEX):
            pipe = abs(x - 32) < 8
            if pipe:
                px[y * TEX + x] = (cel(140), cel(70), cel(40))
            else:
                px[y * TEX + x] = (cel(30), cel(35), cel(45))
    return px


def glass_block():
    px = [0] * (TEX * TEX)
    for y in range(TEX):
        for x in range(TEX):
            ref = (x + y) % 9 == 0
            if ref:
                px[y * TEX + x] = (cel(200), cel(240), cel(255))
            else:
                px[y * TEX + x] = (cel(40), cel(120), cel(180))
    return px


def warning_stripe():
    px = [0] * (TEX * TEX)
    for y in range(TEX):
        for x in range(TEX):
            stripe = ((x + y) // 8) % 2 == 0
            if stripe:
                px[y * TEX + x] = (cel(255), cel(220), cel(0))
            else:
                px[y * TEX + x] = (cel(30), cel(30), cel(30))
    return px


WALLS = {
    "neon_grid": neon_grid,
    "metal_panel": metal_panel,
    "city_lights": city_lights,
    "circuit": circuit,
    "holo_wall": holo_wall,
    "rust_pipe": rust_pipe,
    "glass_block": glass_block,
    "warning_stripe": warning_stripe,
}


def main():
    for name, fn in WALLS.items():
        path = os.path.join(OUT, f"{name}.ppm")
        write_ppm(path, fn())
        print(f"Wrote {path}")


if __name__ == "__main__":
    main()
