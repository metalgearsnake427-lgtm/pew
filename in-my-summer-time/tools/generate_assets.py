#!/usr/bin/env python3
"""Generate enemy/weapon sprite PPMs and 5 sci-fi city level maps."""

import os
import random

TEX = 64
ASSETS = os.path.join(os.path.dirname(__file__), "..", "assets")


def cel(v, steps=5):
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


def sprite_enemy(primary, secondary, shape="humanoid"):
    px = [(0, 0, 0)] * (TEX * TEX)
    cx, cy = TEX // 2, TEX // 2 + 8
    for y in range(TEX):
        for x in range(TEX):
            dx, dy = x - cx, y - cy
            if shape == "drone":
                dist = (dx * dx + (dy + 8) ** 2) ** 0.5
                if dist < 22:
                    px[y * TEX + x] = primary if dist > 18 else secondary
            elif shape == "boss":
                body = abs(dx) < 18 and -20 < dy < 20
                head = dx * dx + (dy + 22) ** 2 < 100
                if head or body:
                    px[y * TEX + x] = secondary if head else primary
            else:
                body = abs(dx) < 12 and -5 < dy < 25
                head = dx * dx + (dy + 18) ** 2 < 64
                gun = 8 < dx < 20 and 0 < dy < 8
                if gun:
                    px[y * TEX + x] = (cel(80), cel(80), cel(90))
                elif head or body:
                    px[y * TEX + x] = secondary if head else primary
    return px


def sprite_weapon(style):
    px = [(0, 0, 0)] * (TEX * TEX)
    for y in range(TEX):
        for x in range(TEX):
            if style == "revolver":
                if 20 < x < 50 and 35 < y < 50:
                    px[y * TEX + x] = (cel(160), cel(160), cel(170))
                if 45 < x < 58 and 38 < y < 46:
                    px[y * TEX + x] = (cel(60), cel(60), cel(70))
            elif style == "shotgun":
                if 10 < x < 55 and 38 < y < 48:
                    px[y * TEX + x] = (cel(100), cel(70), cel(40))
            elif style == "plasma":
                if 15 < x < 55 and 36 < y < 50:
                    px[y * TEX + x] = (cel(40), cel(200), cel(255))
            elif style == "smg":
                if 18 < x < 58 and 40 < y < 48:
                    px[y * TEX + x] = (cel(120), cel(120), cel(130))
            elif style == "sniper":
                if 5 < x < 60 and 42 < y < 46:
                    px[y * TEX + x] = (cel(50), cel(80), cel(50))
            elif style == "dual_pistols":
                if (15 < x < 30 or 35 < x < 50) and 38 < y < 50:
                    px[y * TEX + x] = (cel(180), cel(180), cel(190))
            elif style == "rocket":
                if 8 < x < 58 and 35 < y < 50:
                    px[y * TEX + x] = (cel(180), cel(60), cel(40))
            elif style == "gatling":
                if 12 < x < 58 and 36 < y < 52:
                    px[y * TEX + x] = (cel(90), cel(90), cel(100))
            elif style == "hand_cannon":
                if 22 < x < 48 and 32 < y < 52:
                    px[y * TEX + x] = (cel(200), cel(160), cel(60))
    return px


def carve_maze(w, h, seed):
    random.seed(seed)
    grid = [[1] * w for _ in range(h)]
    stack = [(1, 1)]
    grid[1][1] = 0

    dirs = [(0, -2), (0, 2), (-2, 0), (2, 0)]
    while stack:
        x, y = stack[-1]
        random.shuffle(dirs)
        moved = False
        for dx, dy in dirs:
            nx, ny = x + dx, y + dy
            if 1 <= nx < w - 1 and 1 <= ny < h - 1 and grid[ny][nx] == 1:
                grid[ny][nx] = 0
                grid[y + dy // 2][x + dx // 2] = 0
                stack.append((nx, ny))
                moved = True
                break
        if not moved:
            stack.pop()

    # border walls
    for x in range(w):
        grid[0][x] = grid[h - 1][x] = 1
    for y in range(h):
        grid[y][0] = grid[y][w - 1] = 1

    # sci-fi rooms - open some areas
    for _ in range(w * h // 80):
        rx, ry = random.randrange(2, w - 2), random.randrange(2, h - 2)
        for dy in range(-2, 3):
            for dx in range(-2, 3):
                if 0 <= rx + dx < w and 0 <= ry + dy < h:
                    grid[ry + dy][rx + dx] = 0

    # texture variety on walls
    tex_grid = [[0] * w for _ in range(h)]
    for y in range(h):
        for x in range(w):
            if grid[y][x] == 1:
                tex_grid[y][x] = 1 + (x * 3 + y * 7 + seed) % 8
            else:
                tex_grid[y][x] = 0
    return tex_grid


def find_open_cells(grid):
    cells = []
    h, w = len(grid), len(grid[0])
    for y in range(h):
        for x in range(w):
            if grid[y][x] == 0:
                cells.append((x, y))
    return cells


def write_level(path, w, h, grid, spawn, exit_pos, enemies):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w") as f:
        f.write(f"{w} {h}\n")
        for row in grid:
            f.write(" ".join(str(c) for c in row) + "\n")
        f.write(f"{spawn[0] + 0.5} {spawn[1] + 0.5} {spawn[2]}\n")
        f.write(f"{exit_pos[0] + 0.5} {exit_pos[1] + 0.5}\n")
        f.write(f"{len(enemies)}\n")
        for ex, ey, et in enemies:
            f.write(f"{ex + 0.5} {ey + 0.5} {et}\n")


def generate_levels():
    sizes = [(41, 41), (51, 51), (61, 41), (55, 55), (71, 51)]
    for i, (w, h) in enumerate(sizes):
        grid = carve_maze(w, h, seed=1000 + i * 137)
        open_cells = find_open_cells(grid)
        random.seed(2000 + i)
        spawn = open_cells[0]
        exit_pos = open_cells[-1]
        enemy_count = 12 + i * 6
        enemies = []
        picks = random.sample(open_cells[10:], min(enemy_count, len(open_cells) - 20))
        for j, (ex, ey) in enumerate(picks):
            et = min(3, j // (4 + i))
            enemies.append((ex, ey, et))
        path = os.path.join(ASSETS, "levels", f"level_{i + 1}.map")
        write_level(path, w, h, grid, (spawn[0], spawn[1], 0.0), exit_pos, enemies)
        print(f"Wrote {path} ({w}x{h}, {len(enemies)} enemies)")


def generate_sprites():
    sprites = {
        "drone": sprite_enemy((cel(255), cel(80), cel(120)), (cel(200), cel(200), cel(220)), "drone"),
        "gunner": sprite_enemy((cel(255), cel(180), cel(0)), (cel(80), cel(80), cel(100)), "humanoid"),
        "cyborg": sprite_enemy((cel(0), cel(255), cel(200)), (cel(40), cel(40), cel(60)), "humanoid"),
        "boss": sprite_enemy((cel(255), cel(0), cel(80)), (cel(255), cel(255), cel(0)), "boss"),
    }
    for name, px in sprites.items():
        write_ppm(os.path.join(ASSETS, "sprites", f"{name}.ppm"), px)

    weapons = [
        "revolver", "shotgun", "plasma", "smg", "sniper",
        "dual_pistols", "rocket", "gatling", "hand_cannon",
    ]
    for w in weapons:
        write_ppm(os.path.join(ASSETS, "sprites", f"{w}.ppm"), sprite_weapon(w))


def main():
    generate_sprites()
    generate_levels()


if __name__ == "__main__":
    main()
