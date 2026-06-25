#include "enemy.h"
#include "map.h"
#include <math.h>
#include <stdlib.h>

static float enemy_max_health(int type) {
    switch (type) {
        case 3: return 200.0f;
        case 2: return 80.0f;
        case 1: return 50.0f;
        default: return 35.0f;
    }
}

static float enemy_shot_damage(int type) {
    switch (type) {
        case 3: return 18.0f;
        case 2: return 10.0f;
        case 1: return 8.0f;
        default: return 5.0f;
    }
}

static float enemy_fire_rate(int type) {
    switch (type) {
        case 3: return 0.8f;
        case 2: return 1.2f;
        case 1: return 0.6f;
        default: return 1.0f;
    }
}

void enemy_init_group(EnemyGroup *g, const LevelMap *m) {
    g->count = m->enemy_count;
    for (int i = 0; i < g->count; i++) {
        g->list[i].x = m->enemy_x[i];
        g->list[i].y = m->enemy_y[i];
        g->list[i].type = m->enemy_type[i] % 4;
        g->list[i].alive = 1;
        g->list[i].health = enemy_max_health(g->list[i].type);
        g->list[i].shoot_cd = (float)i * 0.5f;
        g->list[i].anim = 0.0f;
    }
}

static int has_los(const LevelMap *m, float x0, float y0, float x1, float y1) {
    float dx = x1 - x0, dy = y1 - y0;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist < 0.01f) return 1;
    dx /= dist; dy /= dist;
    float step = 0.1f;
    for (float d = step; d < dist; d += step) {
        if (map_wall_at(m, x0 + dx * d, y0 + dy * d)) return 0;
    }
    return 1;
}

void enemy_update(EnemyGroup *g, float dt, float px, float py, float *player_health, const LevelMap *m) {
    for (int i = 0; i < g->count; i++) {
        Enemy *e = &g->list[i];
        if (!e->alive) continue;
        e->anim += dt;
        e->shoot_cd -= dt;

        float dx = px - e->x, dy = py - e->y;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist < 1.0f || dist > 25.0f) continue;
        if (!has_los(m, e->x, e->y, px, py)) continue;

        if (e->shoot_cd <= 0.0f) {
            e->shoot_cd = enemy_fire_rate(e->type);
            float spread = 0.08f + (float)rand() / RAND_MAX * 0.06f;
            float aim = atan2f(dy, dx) + ((float)rand() / RAND_MAX - 0.5f) * spread;
            float adx = cosf(aim), ady = sinf(aim);
            /* simple hit check on player */
            float hit_dist;
            int idx;
            /* check if ray from enemy hits near player */
            float ex = e->x, ey = e->y;
            int blocked = 0;
            for (float d = 0.5f; d < dist + 0.5f; d += 0.15f) {
                float rx = ex + adx * d, ry = ey + ady * d;
                if (map_wall_at(m, rx, ry)) { blocked = 1; break; }
                float pdx = rx - px, pdy = ry - py;
                if (pdx * pdx + pdy * pdy < 0.25f) {
                    *player_health -= enemy_shot_damage(e->type);
                    if (*player_health < 0) *player_health = 0;
                    break;
                }
            }
            (void)blocked;
            (void)hit_dist;
            (void)idx;
        }
    }
}

int enemy_raycast_hit(EnemyGroup *g, float ox, float oy, float dx, float dy, float max_dist, float *hit_dist, int *hit_idx) {
    float best = max_dist;
    int best_i = -1;
    for (int i = 0; i < g->count; i++) {
        if (!g->list[i].alive) continue;
        float ex = g->list[i].x - ox, ey = g->list[i].y - oy;
        /* project enemy center onto ray */
        float t = ex * dx + ey * dy;
        if (t <= 0.05f || t >= best) continue;
        float perp_x = ex - dx * t, perp_y = ey - dy * t;
        float perp = sqrtf(perp_x * perp_x + perp_y * perp_y);
        float radius = 0.4f + g->list[i].type * 0.05f;
        if (perp < radius) {
            best = t;
            best_i = i;
        }
    }
    *hit_dist = best;
    *hit_idx = best_i;
    return best_i >= 0;
}

void enemy_damage(EnemyGroup *g, int idx, float dmg) {
    if (idx < 0 || idx >= g->count) return;
    Enemy *e = &g->list[idx];
    if (!e->alive) return;
    e->health -= dmg;
    if (e->health <= 0.0f) e->alive = 0;
}

int enemy_alive_count(const EnemyGroup *g) {
    int n = 0;
    for (int i = 0; i < g->count; i++)
        if (g->list[i].alive) n++;
    return n;
}
