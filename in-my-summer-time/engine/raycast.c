#include "raycast.h"
#include "texture.h"
#include <math.h>
#include <stdlib.h>

static uint32_t shade_color(uint32_t c, float shade) {
    if (shade < 0.0f) shade = 0.0f;
    if (shade > 1.0f) shade = 1.0f;
    int r = (int)(((c >> 16) & 0xFF) * shade);
    int g = (int)(((c >> 8) & 0xFF) * shade);
    int b = (int)((c & 0xFF) * shade);
    return 0xFF000000u | (r << 16) | (g << 8) | b;
}

static void cast_ray(const LevelMap *m, float px, float py, float ray_dir_x, float ray_dir_y, RayHit *hit) {
    int map_x = (int)px;
    int map_y = (int)py;

    float delta_dist_x = fabsf(1.0f / ray_dir_x);
    float delta_dist_y = fabsf(1.0f / ray_dir_y);

    int step_x, step_y;
    float side_dist_x, side_dist_y;

    if (ray_dir_x < 0) { step_x = -1; side_dist_x = (px - map_x) * delta_dist_x; }
    else { step_x = 1; side_dist_x = (map_x + 1.0f - px) * delta_dist_x; }

    if (ray_dir_y < 0) { step_y = -1; side_dist_y = (py - map_y) * delta_dist_y; }
    else { step_y = 1; side_dist_y = (map_y + 1.0f - py) * delta_dist_y; }

    int side = 0;
    int hit_wall = 0;

    for (int i = 0; i < 256 && !hit_wall; i++) {
        if (side_dist_x < side_dist_y) {
            side_dist_x += delta_dist_x;
            map_x += step_x;
            side = 0;
        } else {
            side_dist_y += delta_dist_y;
            map_y += step_y;
            side = 1;
        }
        if (map_x < 0 || map_y < 0 || map_x >= m->w || map_y >= m->h ||
            m->tiles[map_y * m->w + map_x] > 0) {
            hit_wall = 1;
        }
    }

    float perp_dist;
    if (side == 0)
        perp_dist = (map_x - px + (1 - step_x) / 2.0f) / ray_dir_x;
    else
        perp_dist = (map_y - py + (1 - step_y) / 2.0f) / ray_dir_y;

    if (perp_dist < 0.01f) perp_dist = 0.01f;

    float wall_x;
    if (side == 0) wall_x = py + perp_dist * ray_dir_y;
    else wall_x = px + perp_dist * ray_dir_x;
    wall_x -= floorf(wall_x);

    int tex = 0;
    if (map_x >= 0 && map_y >= 0 && map_x < m->w && map_y < m->h) {
        int v = m->tiles[map_y * m->w + map_x];
        tex = v > 0 ? (v - 1) % NUM_WALL_TEX : 0;
    }

    hit->dist = perp_dist;
    hit->side = side;
    hit->wall_x = wall_x;
    hit->tex_id = tex;
}

static void draw_wall_column(uint32_t *fb, int x, const RayHit *hit) {
    int line_h = (int)(SCREEN_H / hit->dist);
    if (line_h > SCREEN_H * 3) line_h = SCREEN_H * 3;
    int draw_start = -line_h / 2 + SCREEN_H / 2;
    int draw_end = line_h / 2 + SCREEN_H / 2;
    if (draw_start < 0) draw_start = 0;
    if (draw_end >= SCREEN_H) draw_end = SCREEN_H - 1;

    float shade = 1.0f / (1.0f + hit->dist * 0.08f);
    if (hit->side) shade *= 0.75f;

    Texture *tex = &wall_tex[hit->tex_id];
    int tex_x = (int)(hit->wall_x * TEX_SIZE);
    if (tex_x < 0) tex_x = 0;
    if (tex_x >= TEX_SIZE) tex_x = TEX_SIZE - 1;

    for (int y = draw_start; y <= draw_end; y++) {
        int d = y * 256 - SCREEN_H * 128 + line_h * 128;
        int tex_y = ((d * TEX_SIZE) / line_h) / 256;
        if (tex_y < 0) tex_y = 0;
        if (tex_y >= TEX_SIZE) tex_y = TEX_SIZE - 1;
        uint32_t color = tex->pixels[tex_y * TEX_SIZE + tex_x];
        fb[y * SCREEN_W + x] = shade_color(color, shade);
    }
}

typedef struct {
    float dist;
    int idx;
    int type;
} SpriteSort;

static int sprite_cmp(const void *a, const void *b) {
    float da = ((const SpriteSort *)a)->dist;
    float db = ((const SpriteSort *)b)->dist;
    return (da > db) - (da < db);
}

static void draw_sprites(uint32_t *fb, float px, float py, float pa,
                         const EnemyGroup *enemies) {
    SpriteSort order[MAX_ENEMIES];
    int n = 0;
    for (int i = 0; i < enemies->count; i++) {
        if (!enemies->list[i].alive) continue;
        float dx = enemies->list[i].x - px;
        float dy = enemies->list[i].y - py;
        order[n].dist = dx * dx + dy * dy;
        order[n].idx = i;
        order[n].type = enemies->list[i].type;
        n++;
    }
    qsort(order, n, sizeof(SpriteSort), sprite_cmp);

    float plane_x = cosf(pa + 1.5707963f) * FOV;
    float plane_y = sinf(pa + 1.5707963f) * FOV;
    float dir_x = cosf(pa), dir_y = sinf(pa);

    for (int s = 0; s < n; s++) {
        int i = order[s].idx;
        float sx = enemies->list[i].x - px;
        float sy = enemies->list[i].y - py;

        float inv_det = 1.0f / (plane_x * dir_y - dir_x * plane_y);
        float transform_x = inv_det * (dir_y * sx - dir_x * sy);
        float transform_y = inv_det * (-plane_y * sx + plane_x * sy);

        if (transform_y <= 0.1f) continue;

        int sprite_screen_x = (int)((SCREEN_W / 2) * (1 + transform_x / transform_y));
        int sprite_h = abs((int)(SCREEN_H / transform_y));
        int draw_start_y = -sprite_h / 2 + SCREEN_H / 2;
        int draw_end_y = sprite_h / 2 + SCREEN_H / 2;
        int sprite_w = sprite_h;
        int draw_start_x = -sprite_w / 2 + sprite_screen_x;
        int draw_end_x = sprite_w / 2 + sprite_screen_x;

        if (draw_start_y < 0) draw_start_y = 0;
        if (draw_end_y >= SCREEN_H) draw_end_y = SCREEN_H - 1;
        if (draw_start_x < 0) draw_start_x = 0;
        if (draw_end_x >= SCREEN_W) draw_end_x = SCREEN_W - 1;

        int tex_id = enemies->list[i].type % NUM_SPRITES;
        Texture *tex = &sprite_tex[tex_id];
        float shade = 1.0f / (1.0f + transform_y * 0.12f);

        for (int stripe = draw_start_x; stripe < draw_end_x; stripe++) {
            int tex_x = (int)(256 * (stripe - (-sprite_w / 2 + sprite_screen_x)) * TEX_SIZE / sprite_w) / 256;
            if (tex_x < 0 || tex_x >= TEX_SIZE) continue;

            for (int y = draw_start_y; y < draw_end_y; y++) {
                int d = y * 256 - SCREEN_H * 128 + sprite_h * 128;
                int tex_y = ((d * TEX_SIZE) / sprite_h) / 256;
                if (tex_y < 0 || tex_y >= TEX_SIZE) continue;
                uint32_t color = tex->pixels[tex_y * TEX_SIZE + tex_x];
                if ((color & 0xFF000000u) == 0) continue;
                if ((color & 0x00FFFFFFu) < 0x00101010u) continue;
                fb[y * SCREEN_W + stripe] = shade_color(color, shade);
            }
        }
    }
}

void raycast_render(uint32_t *fb, const LevelMap *m, float px, float py, float pa,
                    const EnemyGroup *enemies) {
    /* sky gradient - sci-fi purple/pink */
    for (int y = 0; y < SCREEN_H / 2; y++) {
        float t = (float)y / (SCREEN_H / 2);
        int r = (int)(26 + t * 80);
        int g = (int)(10 + t * 20);
        int b = (int)(46 + t * 100);
        uint32_t c = 0xFF000000u | (r << 16) | (g << 8) | b;
        for (int x = 0; x < SCREEN_W; x++)
            fb[y * SCREEN_W + x] = c;
    }
    /* floor */
    for (int y = SCREEN_H / 2; y < SCREEN_H; y++) {
        float t = (float)(y - SCREEN_H / 2) / (SCREEN_H / 2);
        int r = (int)(13 + t * 5);
        int g = (int)(17 + t * 8);
        int b = (int)(23 + t * 12);
        uint32_t c = 0xFF000000u | (r << 16) | (g << 8) | b;
        for (int x = 0; x < SCREEN_W; x++)
            fb[y * SCREEN_W + x] = c;
    }

    float dir_x = cosf(pa), dir_y = sinf(pa);
    float plane_x = cosf(pa + 1.5707963f) * FOV;
    float plane_y = sinf(pa + 1.5707963f) * FOV;

    for (int x = 0; x < SCREEN_W; x++) {
        float camera_x = 2.0f * x / SCREEN_W - 1.0f;
        float ray_dir_x = dir_x + plane_x * camera_x;
        float ray_dir_y = dir_y + plane_y * camera_x;

        RayHit hit;
        cast_ray(m, px, py, ray_dir_x, ray_dir_y, &hit);
        draw_wall_column(fb, x, &hit);
    }

    draw_sprites(fb, px, py, pa, enemies);
}
