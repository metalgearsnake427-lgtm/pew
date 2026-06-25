#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LevelMap levels[NUM_LEVELS];

int map_is_wall(const LevelMap *m, int tx, int ty) {
    if (tx < 0 || ty < 0 || tx >= m->w || ty >= m->h) return 1;
    return m->tiles[ty * m->w + tx] > 0;
}

int map_wall_at(const LevelMap *m, float x, float y) {
    return map_is_wall(m, (int)x, (int)y);
}

float map_tex_id(const LevelMap *m, float x, float y) {
    int tx = (int)x, ty = (int)y;
    if (tx < 0 || ty < 0 || tx >= m->w || ty >= m->h) return 1.0f;
    int v = m->tiles[ty * m->w + tx];
    return v > 0 ? (float)((v - 1) % NUM_WALL_TEX) : 1.0f;
}

int map_load(LevelMap *m, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    memset(m, 0, sizeof(*m));
    if (fscanf(f, "%d %d", &m->w, &m->h) != 2) { fclose(f); return -1; }
    if (m->w > MAX_MAP || m->h > MAX_MAP) { fclose(f); return -1; }

    for (int y = 0; y < m->h; y++)
        for (int x = 0; x < m->w; x++)
            if (fscanf(f, "%d", &m->tiles[y * m->w + x]) != 1) {
                fclose(f); return -1;
            }

    if (fscanf(f, "%f %f %f", &m->spawn_x, &m->spawn_y, &m->spawn_angle) != 3) {
        fclose(f); return -1;
    }
    if (fscanf(f, "%f %f", &m->exit_x, &m->exit_y) != 2) {
        fclose(f); return -1;
    }
    if (fscanf(f, "%d", &m->enemy_count) != 1) {
        fclose(f); return -1;
    }
    if (m->enemy_count > MAX_ENEMIES) m->enemy_count = MAX_ENEMIES;

    for (int i = 0; i < m->enemy_count; i++) {
        if (fscanf(f, "%f %f %d", &m->enemy_x[i], &m->enemy_y[i], &m->enemy_type[i]) != 3) {
            fclose(f); return -1;
        }
    }
    fclose(f);
    return 0;
}

void map_load_all(const char *asset_dir) {
    char path[512];
    for (int i = 0; i < NUM_LEVELS; i++) {
        snprintf(path, sizeof(path), "%s/levels/level_%d.map", asset_dir, i + 1);
        if (map_load(&levels[i], path) != 0) {
            /* minimal fallback level */
            LevelMap *m = &levels[i];
            memset(m, 0, sizeof(*m));
            m->w = 16; m->h = 16;
            for (int y = 0; y < 16; y++)
                for (int x = 0; x < 16; x++)
                    m->tiles[y * 16 + x] = (x == 0 || y == 0 || x == 15 || y == 15) ? 1 : 0;
            m->spawn_x = 2.5f; m->spawn_y = 2.5f; m->spawn_angle = 0.0f;
            m->exit_x = 13.5f; m->exit_y = 13.5f;
            m->enemy_count = 3;
            m->enemy_x[0] = 8.5f; m->enemy_y[0] = 8.5f; m->enemy_type[0] = 0;
            m->enemy_x[1] = 10.5f; m->enemy_y[1] = 5.5f; m->enemy_type[1] = 1;
            m->enemy_x[2] = 5.5f; m->enemy_y[2] = 10.5f; m->enemy_type[2] = 0;
        }
    }
}
