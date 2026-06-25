#ifndef ENEMY_H
#define ENEMY_H

#include "constants.h"
#include "map.h"

typedef struct {
    float x, y;
    int type;
    int alive;
    float health;
    float shoot_cd;
    float anim;
} Enemy;

typedef struct {
    Enemy list[MAX_ENEMIES];
    int count;
} EnemyGroup;

void enemy_init_group(EnemyGroup *g, const LevelMap *m);
void enemy_update(EnemyGroup *g, float dt, float px, float py, float *player_health, const LevelMap *m);
int enemy_raycast_hit(EnemyGroup *g, float ox, float oy, float dx, float dy, float max_dist, float *hit_dist, int *hit_idx);
void enemy_damage(EnemyGroup *g, int idx, float dmg);
int enemy_alive_count(const EnemyGroup *g);

#endif
