#ifndef GAME_H
#define GAME_H

#include "constants.h"
#include "map.h"
#include "enemy.h"
#include "weapon.h"
#include "input.h"
#include "render.h"

typedef struct {
    int level_idx;
    float px, py, pa;
    float health;
    LevelMap *current_map;
    EnemyGroup enemies;
    WeaponState weapons;
    InputState input;
    RenderContext render;
    int won; /* 0 playing, 1 level clear pause, 2 game won */
    float level_clear_timer;
    int running;
} Game;

LevelMap *game_current_map(void);
int game_init(Game *g, const char *asset_dir, int scale);
void game_shutdown(Game *g);
void game_load_level(Game *g, int idx);
void game_update(Game *g, float dt);
void game_run(Game *g);

#endif
