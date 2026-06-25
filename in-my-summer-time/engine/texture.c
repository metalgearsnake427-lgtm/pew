#include "texture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Texture wall_tex[NUM_WALL_TEX];
Texture sprite_tex[NUM_SPRITES];
Texture weapon_tex[NUM_WEAPONS];

static uint32_t ppm_to_rgba(int r, int g, int b) {
    return 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

int texture_load_ppm(Texture *tex, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    char magic[3];
    if (fscanf(f, "%2s", magic) != 1 || magic[0] != 'P' || magic[1] != '6') {
        fclose(f);
        return -1;
    }

    int w, h, maxval;
    if (fscanf(f, "%d %d %d", &w, &h, &maxval) != 3 || w != TEX_SIZE || h != TEX_SIZE) {
        fclose(f);
        return -1;
    }
    fgetc(f); /* whitespace after header */

    for (int y = 0; y < TEX_SIZE; y++) {
        for (int x = 0; x < TEX_SIZE; x++) {
            int r = fgetc(f), g = fgetc(f), b = fgetc(f);
            if (r == EOF) { fclose(f); return -1; }
            tex->pixels[y * TEX_SIZE + x] = ppm_to_rgba(r, g, b);
        }
    }
    fclose(f);
    return 0;
}

static void make_fallback(Texture *tex, uint32_t c1, uint32_t c2) {
    for (int y = 0; y < TEX_SIZE; y++) {
        for (int x = 0; x < TEX_SIZE; x++) {
            tex->pixels[y * TEX_SIZE + x] = ((x / 8 + y / 8) & 1) ? c1 : c2;
        }
    }
}

void texture_load_all(const char *asset_dir) {
    char path[512];
    const char *wall_names[] = {
        "neon_grid", "metal_panel", "city_lights", "circuit",
        "holo_wall", "rust_pipe", "glass_block", "warning_stripe"
    };
    const char *sprite_names[] = {"drone", "gunner", "cyborg", "boss"};
    const char *weapon_names[] = {
        "revolver", "shotgun", "plasma", "smg", "sniper",
        "dual_pistols", "rocket", "gatling", "hand_cannon"
    };

    for (int i = 0; i < NUM_WALL_TEX; i++) {
        snprintf(path, sizeof(path), "%s/textures/%s.ppm", asset_dir, wall_names[i]);
        if (texture_load_ppm(&wall_tex[i], path) != 0)
            make_fallback(&wall_tex[i], 0xFF00FFFF, 0xFF220044);
    }
    for (int i = 0; i < NUM_SPRITES; i++) {
        snprintf(path, sizeof(path), "%s/sprites/%s.ppm", asset_dir, sprite_names[i]);
        if (texture_load_ppm(&sprite_tex[i], path) != 0)
            make_fallback(&sprite_tex[i], 0xFFFF0055, 0xFF000000);
    }
    for (int i = 0; i < NUM_WEAPONS; i++) {
        snprintf(path, sizeof(path), "%s/sprites/%s.ppm", asset_dir, weapon_names[i]);
        if (texture_load_ppm(&weapon_tex[i], path) != 0)
            make_fallback(&weapon_tex[i], 0xFFCCCCCC, 0xFF444444);
    }
}
