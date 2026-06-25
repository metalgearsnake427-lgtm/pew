#include "render.h"
#include "constants.h"
#include "texture.h"
#include "map.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

int render_init(RenderContext *rc, int scale) {
    memset(rc, 0, sizeof(*rc));
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return -1;

    rc->window = SDL_CreateWindow(
        "in my summer time, all alone",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W * scale, SCREEN_H * scale,
        SDL_WINDOW_SHOWN);
    if (!rc->window) return -1;

    rc->renderer = SDL_CreateRenderer(rc->window, -1, SDL_RENDERER_ACCELERATED);
    if (!rc->renderer) {
        rc->renderer = SDL_CreateRenderer(rc->window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!rc->renderer) return -1;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    rc->screen_tex = SDL_CreateTexture(rc->renderer, SDL_PIXELFORMAT_ARGB8888,
                                       SDL_TEXTUREACCESS_STREAMING, SCREEN_W, SCREEN_H);
    if (!rc->screen_tex) return -1;

    rc->framebuffer = calloc(SCREEN_W * SCREEN_H, sizeof(uint32_t));
    return rc->framebuffer ? 0 : -1;
}

void render_shutdown(RenderContext *rc) {
    free(rc->framebuffer);
    if (rc->screen_tex) SDL_DestroyTexture(rc->screen_tex);
    if (rc->renderer) SDL_DestroyRenderer(rc->renderer);
    if (rc->window) SDL_DestroyWindow(rc->window);
    SDL_Quit();
}

void render_present(RenderContext *rc) {
    SDL_UpdateTexture(rc->screen_tex, NULL, rc->framebuffer, SCREEN_W * sizeof(uint32_t));
    SDL_RenderClear(rc->renderer);
    SDL_RenderCopy(rc->renderer, rc->screen_tex, NULL, NULL);
    SDL_RenderPresent(rc->renderer);
}

static void draw_rect(uint32_t *fb, int x, int y, int w, int h, uint32_t color) {
    for (int j = y; j < y + h && j < SCREEN_H; j++) {
        if (j < 0) continue;
        for (int i = x; i < x + w && i < SCREEN_W; i++) {
            if (i < 0) continue;
            fb[j * SCREEN_W + i] = color;
        }
    }
}

static void blit_tex(uint32_t *fb, const Texture *tex, int dx, int dy, int dw, int dh, float alpha) {
    for (int y = 0; y < dh; y++) {
        int sy = y * TEX_SIZE / dh;
        int fy = dy + y;
        if (fy < 0 || fy >= SCREEN_H) continue;
        for (int x = 0; x < dw; x++) {
            int sx = x * TEX_SIZE / dw;
            int fx = dx + x;
            if (fx < 0 || fx >= SCREEN_W) continue;
            uint32_t src = tex->pixels[sy * TEX_SIZE + sx];
            if ((src & 0xFF000000u) == 0) continue;
            if ((src & 0x00FFFFFFu) < 0x00080808u) continue;
            uint32_t *dst = &fb[fy * SCREEN_W + fx];
            if (alpha >= 1.0f) {
                *dst = src;
            } else {
                int sr = (src >> 16) & 0xFF, sg = (src >> 8) & 0xFF, sb = src & 0xFF;
                int dr = (*dst >> 16) & 0xFF, dg = (*dst >> 8) & 0xFF, db = *dst & 0xFF;
                int r = (int)(dr + (sr - dr) * alpha);
                int g = (int)(dg + (sg - dg) * alpha);
                int b = (int)(db + (sb - db) * alpha);
                *dst = 0xFF000000u | (r << 16) | (g << 8) | b;
            }
        }
    }
}

void render_hud(RenderContext *rc, float health, int weapon_idx, const WeaponState *ws,
                int level, int enemies_left, int game_won) {
    uint32_t *fb = rc->framebuffer;

    /* health bar - stylized neon */
    int bar_w = 80, bar_h = 8;
    draw_rect(fb, 8, 8, bar_w, bar_h, 0xFF220033);
    int fill = (int)(bar_w * (health / MAX_HEALTH));
    if (fill > bar_w) fill = bar_w;
    uint32_t hp_color = health > 30 ? 0xFF00FFAA : 0xFFFF0055;
    draw_rect(fb, 8, 8, fill, bar_h, hp_color);
    draw_rect(fb, 8, 8, bar_w, 1, 0xFFFF00FF);
    draw_rect(fb, 8, 15, bar_w, 1, 0xFFFF00FF);

    /* weapon name strip */
    char buf[64];
    snprintf(buf, sizeof(buf), "LV%d  %s  [%d left]", level + 1, weapons[weapon_idx].name, enemies_left);
    /* simple text via pixel blocks omitted - use weapon icon area */

    /* weapon sprite HUD */
    int bob = (int)(sinf(ws->bob) * 4.0f);
    int ww = 90, wh = 60;
    int wx = SCREEN_W / 2 - ww / 2;
    int wy = SCREEN_H - wh - 8 + bob;
    blit_tex(fb, &weapon_tex[weapon_idx], wx, wy, ww, wh, 1.0f);

    /* muzzle flash overlay */
    if (ws->flash > 0.0f) {
        uint32_t flash = 0x60FFAA00;
        for (int y = SCREEN_H / 2 - 20; y < SCREEN_H / 2 + 20; y++)
            for (int x = SCREEN_W / 2 - 30; x < SCREEN_W / 2 + 30; x++)
                if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H)
                    fb[y * SCREEN_W + x] = flash;
    }

    /* crosshair */
    uint32_t ch = 0xFFFF00FF;
    draw_rect(fb, SCREEN_W / 2 - 6, SCREEN_H / 2, 12, 1, ch);
    draw_rect(fb, SCREEN_W / 2, SCREEN_H / 2 - 6, 1, 12, ch);

    /* level complete / win overlay */
    if (game_won == 1) {
        draw_rect(fb, SCREEN_W / 4, SCREEN_H / 3, SCREEN_W / 2, SCREEN_H / 3, 0xC0000044);
    } else if (game_won == 2) {
        draw_rect(fb, SCREEN_W / 6, SCREEN_H / 3, SCREEN_W * 2 / 3, SCREEN_H / 3, 0xC000AA00);
    }

    (void)buf;
}

void render_minimap(uint32_t *fb, const LevelMap *m, float px, float py, float pa) {
    int mm_w = 56, mm_h = 56;
    int ox = SCREEN_W - mm_w - 6, oy = 6;
    float scale_x = (float)mm_w / (float)m->w;
    float scale_y = (float)mm_h / (float)m->h;

    draw_rect(fb, ox - 1, oy - 1, mm_w + 2, mm_h + 2, 0xFFFF00FF);
    for (int my = 0; my < mm_h; my++) {
        for (int mx = 0; mx < mm_w; mx++) {
            int tx = (int)(mx / scale_x);
            int ty = (int)(my / scale_y);
            uint32_t c = 0xFF111122;
            if (tx >= 0 && ty >= 0 && tx < m->w && ty < m->h) {
                if (m->tiles[ty * m->w + tx] > 0)
                    c = 0xFF004466;
            }
            fb[(oy + my) * SCREEN_W + ox + mx] = c;
        }
    }
    /* exit portal */
    int ex = ox + (int)(m->exit_x * scale_x);
    int ey = oy + (int)(m->exit_y * scale_y);
    draw_rect(fb, ex - 1, ey - 1, 3, 3, 0xFF00FFAA);
    /* player */
    int plx = ox + (int)(px * scale_x);
    int ply = oy + (int)(py * scale_y);
    draw_rect(fb, plx, ply, 2, 2, 0xFFFF0055);
    /* facing line */
    int fx = plx + (int)(cosf(pa) * 6);
    int fy = ply + (int)(sinf(pa) * 6);
    draw_rect(fb, fx, fy, 2, 2, 0xFFFF00FF);
}
