#ifndef TEXTURE_H
#define TEXTURE_H

#include "constants.h"
#include <stdint.h>

typedef struct {
    uint32_t pixels[TEX_SIZE * TEX_SIZE];
} Texture;

extern Texture wall_tex[NUM_WALL_TEX];
extern Texture sprite_tex[NUM_SPRITES];
extern Texture weapon_tex[NUM_WEAPONS];

int texture_load_ppm(Texture *tex, const char *path);
void texture_load_all(const char *asset_dir);

#endif
