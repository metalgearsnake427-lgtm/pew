#ifndef RAYCAST_H
#define RAYCAST_H

#include "constants.h"
#include "map.h"
#include "enemy.h"
#include <stdint.h>

typedef struct {
    float dist;
    int side;
    float wall_x;
    int tex_id;
} RayHit;

void raycast_render(uint32_t *fb, const LevelMap *m, float px, float py, float pa,
                    const EnemyGroup *enemies);

#endif
