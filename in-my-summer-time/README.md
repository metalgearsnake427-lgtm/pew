# in my summer time, all alone

Sci-fi city raycasting FPS — Wolf3D-style engine with bold cel-shaded textures, 9 guns, shooting enemies, and 5 large levels.

## Requirements

- GCC or Clang
- SDL2 development libraries (`sdl2`, `sdl2-config`)
- Python 3
- Rust/Cargo (optional, for map validation)
- GNU Make

### Arch Linux

```bash
sudo pacman -S sdl2 gcc make python
```

### Debian/Ubuntu

```bash
sudo apt install libsdl2-dev build-essential python3
```

## Build & Run

```bash
cd in-my-summer-time
make          # generates assets, validates maps, builds game
./summer      # run from project root
```

Or with custom asset path / scale:

```bash
./summer assets 4    # 4x screen scale
```

## Controls

| Input | Action |
|-------|--------|
| W/A/S/D | Move / strafe |
| Mouse | Look |
| Left click | Shoot |
| 1–9 | Switch weapon |
| Shift | Run |
| Esc | Quit |
| N | Skip to next level (debug) |

Reach the **exit portal** (far corner of each map) after killing all enemies to advance.

## Project Layout

```
in-my-summer-time/
├── engine/           C raycasting engine (SDL2)
├── tools/            Python asset & level generators
├── rust/             Map validator utility
├── assets/           Generated textures, sprites, levels
├── Makefile
└── README.md
```

## Weapons

1. Revolver — precise, high damage
2. Shotgun — 8-pellet spread
3. Plasma Rifle — fast medium damage
4. SMG — high rate, low damage
5. Sniper — extreme damage, tight spread
6. Dual Pistols — balanced sidearms
7. Rocket — massive single-shot damage
8. Gatling — fastest fire rate
9. Hand Cannon — slow heavy hitter

All weapons have unlimited ammo.

## Architecture

- **Engine (C)**: DDA raycasting, sprite billboards, delta-time movement with sliding collision, relative mouse capture
- **Tools (Python)**: Procedural sci-fi PPM textures, maze levels, enemy/weapon sprites
- **Rust**: `validate_maps` checks spawn/exit/enemy placement

## Known Limitations

- No sound yet
- Sprite depth sorting is approximate (no full z-buffer)
- HUD text is minimal (weapon shown via sprite icon)
- Software raycasting only — authentic but not GPU-accelerated 3D

## License

Source provided for learning and modification.
