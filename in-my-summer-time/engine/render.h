#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include "weapon.h"
#include "map.h"
#include <stdint.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *screen_tex;
    uint32_t *framebuffer;
} RenderContext;

int render_init(RenderContext *rc, int scale);
void render_shutdown(RenderContext *rc);
void render_present(RenderContext *rc);
void render_hud(RenderContext *rc, float health, int weapon_idx, const WeaponState *ws,
                int level, int enemies_left, int game_won);
void render_minimap(uint32_t *fb, const LevelMap *m, float px, float py, float pa);

#endif
