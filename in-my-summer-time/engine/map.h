#ifndef MAP_H
#define MAP_H

#include "constants.h"

typedef struct {
    int w, h;
    int tiles[MAX_MAP * MAX_MAP];
    float spawn_x, spawn_y, spawn_angle;
    float exit_x, exit_y;
    int enemy_count;
    float enemy_x[MAX_ENEMIES];
    float enemy_y[MAX_ENEMIES];
    int enemy_type[MAX_ENEMIES];
} LevelMap;

extern LevelMap levels[NUM_LEVELS];

int map_load(LevelMap *m, const char *path);
int map_is_wall(const LevelMap *m, int tx, int ty);
int map_wall_at(const LevelMap *m, float x, float y);
float map_tex_id(const LevelMap *m, float x, float y);
void map_load_all(const char *asset_dir);

#endif
