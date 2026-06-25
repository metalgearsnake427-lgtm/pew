//! Validates level map files for the raycasting engine.

use std::env;
use std::fs;
use std::path::Path;

#[derive(Debug)]
struct LevelMap {
    w: usize,
    h: usize,
    tiles: Vec<i32>,
    spawn: (f32, f32, f32),
    exit: (f32, f32),
    enemies: Vec<(f32, f32, i32)>,
}

fn parse_map(path: &Path) -> Result<LevelMap, String> {
    let text = fs::read_to_string(path).map_err(|e| e.to_string())?;
    let mut lines = text.lines().filter(|l| !l.trim().is_empty());
    let wh: Vec<&str> = lines
        .next()
        .ok_or("missing dimensions")?
        .split_whitespace()
        .collect();
    let w: usize = wh[0].parse().map_err(|_| "bad width")?;
    let h: usize = wh[1].parse().map_err(|_| "bad height")?;
    let mut tiles = Vec::with_capacity(w * h);
    for _ in 0..h {
        let row: Vec<i32> = lines
            .next()
            .ok_or("short grid")?
            .split_whitespace()
            .map(|s| s.parse().unwrap_or(-1))
            .collect();
        if row.len() != w {
            return Err(format!("row width mismatch in {}", path.display()));
        }
        tiles.extend(row);
    }
    let spawn_p: Vec<f32> = lines
        .next()
        .ok_or("missing spawn")?
        .split_whitespace()
        .map(|s| s.parse().unwrap_or(0.0))
        .collect();
    let exit_p: Vec<f32> = lines
        .next()
        .ok_or("missing exit")?
        .split_whitespace()
        .map(|s| s.parse().unwrap_or(0.0))
        .collect();
    let enemy_count: usize = lines.next().ok_or("missing enemy count")?.parse().unwrap_or(0);
    let mut enemies = Vec::new();
    for _ in 0..enemy_count {
        let e: Vec<f32> = lines
            .next()
            .ok_or("short enemy list")?
            .split_whitespace()
            .map(|s| s.parse().unwrap_or(0.0))
            .collect();
        enemies.push((e[0], e[1], e.get(2).copied().unwrap_or(0.0) as i32));
    }
    Ok(LevelMap {
        w,
        h,
        tiles,
        spawn: (spawn_p[0], spawn_p[1], spawn_p.get(2).copied().unwrap_or(0.0)),
        exit: (exit_p[0], exit_p[1]),
        enemies,
    })
}

fn tile_at(m: &LevelMap, x: i32, y: i32) -> i32 {
    if x < 0 || y < 0 || x as usize >= m.w || y as usize >= m.h {
        return 1;
    }
    m.tiles[y as usize * m.w + x as usize]
}

fn validate(m: &LevelMap, name: &str) -> Vec<String> {
    let mut errors = Vec::new();
    if m.w < 8 || m.h < 8 {
        errors.push(format!("{name}: map too small"));
    }
    let sx = m.spawn.0 as i32;
    let sy = m.spawn.1 as i32;
    if tile_at(m, sx, sy) != 0 {
        errors.push(format!("{name}: spawn inside wall"));
    }
    let ex = m.exit.0 as i32;
    let ey = m.exit.1 as i32;
    if tile_at(m, ex, ey) != 0 {
        errors.push(format!("{name}: exit inside wall"));
    }
    if m.enemies.is_empty() {
        errors.push(format!("{name}: no enemies"));
    }
    for (i, (x, y, t)) in m.enemies.iter().enumerate() {
        if tile_at(m, *x as i32, *y as i32) != 0 {
            errors.push(format!("{name}: enemy {i} inside wall"));
        }
        if *t < 0 || *t > 3 {
            errors.push(format!("{name}: enemy {i} bad type {t}"));
        }
    }
    errors
}

fn main() {
    let dir = env::args()
        .nth(1)
        .unwrap_or_else(|| "../assets/levels".to_string());
    let path = Path::new(&dir);
    let mut total_errors = 0;
    for entry in fs::read_dir(path).expect("read levels dir") {
        let entry = entry.expect("entry");
        let p = entry.path();
        if p.extension().and_then(|e| e.to_str()) != Some("map") {
            continue;
        }
        match parse_map(&p) {
            Ok(m) => {
                let name = p.file_name().unwrap().to_string_lossy().to_string();
                let errs = validate(&m, &name);
                if errs.is_empty() {
                    println!(
                        "OK {} ({}x{}, {} enemies)",
                        name,
                        m.w,
                        m.h,
                        m.enemies.len()
                    );
                } else {
                    for e in &errs {
                        eprintln!("FAIL: {e}");
                    }
                    total_errors += errs.len();
                }
            }
            Err(e) => {
                eprintln!("FAIL {}: {e}", p.display());
                total_errors += 1;
            }
        }
    }
    if total_errors > 0 {
        std::process::exit(1);
    }
    println!("All maps valid.");
}
