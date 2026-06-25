#include "weapon.h"
#include <math.h>
#include <stdlib.h>

WeaponDef weapons[NUM_WEAPONS] = {
    {"Revolver",       0.45f, 35.0f, 0.02f, 20.0f, 1, 0},
    {"Shotgun",        0.85f, 12.0f, 0.18f, 12.0f, 8, 1},
    {"Plasma Rifle",   0.12f, 14.0f, 0.04f, 18.0f, 1, 2},
    {"SMG",            0.07f,  8.0f, 0.08f, 16.0f, 1, 3},
    {"Sniper",         1.20f, 80.0f, 0.005f, 30.0f, 1, 4},
    {"Dual Pistols",   0.20f, 18.0f, 0.05f, 18.0f, 1, 5},
    {"Rocket",         1.50f, 120.0f, 0.01f, 25.0f, 1, 6},
    {"Gatling",        0.05f,  6.0f, 0.10f, 15.0f, 1, 7},
    {"Hand Cannon",    1.00f, 55.0f, 0.03f, 22.0f, 1, 8},
};

void weapon_init(WeaponState *ws) {
    ws->current = 0;
    ws->cooldown = 0.0f;
    ws->bob = 0.0f;
    ws->firing = 0;
    ws->flash = 0.0f;
}

void weapon_switch(WeaponState *ws, int idx) {
    if (idx >= 0 && idx < NUM_WEAPONS) {
        ws->current = idx;
        ws->cooldown = 0.0f;
    }
}

int weapon_fire(WeaponState *ws, float px, float py, float angle,
                void (*hit_callback)(float ox, float oy, float ray_angle, float dmg, void *ctx),
                void *ctx) {
    if (ws->cooldown > 0.0f) return 0;

    WeaponDef *w = &weapons[ws->current];
    ws->cooldown = w->fire_rate;
    ws->firing = 1;
    ws->flash = 0.15f;

    for (int p = 0; p < w->pellets; p++) {
        float spread = ((float)rand() / RAND_MAX - 0.5f) * w->spread;
        float a = angle + spread;
        if (hit_callback)
            hit_callback(px, py, a, w->damage, ctx);
    }
    return 1;
}
