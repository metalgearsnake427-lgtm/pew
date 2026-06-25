#ifndef WEAPON_H
#define WEAPON_H

#include "constants.h"

typedef struct {
    const char *name;
    float fire_rate;
    float damage;
    float spread;
    float range;
    int pellets;
    int tex_id;
} WeaponDef;

extern WeaponDef weapons[NUM_WEAPONS];

typedef struct {
    int current;
    float cooldown;
    float bob;
    int firing;
    float flash;
} WeaponState;

void weapon_init(WeaponState *ws);
void weapon_switch(WeaponState *ws, int idx);
int weapon_fire(WeaponState *ws, float px, float py, float angle,
                void (*hit_callback)(float ox, float oy, float angle, float dmg, void *ctx),
                void *ctx);

#endif
