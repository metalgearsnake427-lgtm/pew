#include "game.h"
#include "raycast.h"
#include "texture.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static Game *g_singleton = NULL;

LevelMap *game_current_map(void) {
    return g_singleton ? g_singleton->current_map : NULL;
}

static int collision_at(const LevelMap *m, float x, float y) {
    /* check four corners of player radius */
    float r = PLAYER_RADIUS;
    if (map_wall_at(m, x - r, y - r)) return 1;
    if (map_wall_at(m, x + r, y - r)) return 1;
    if (map_wall_at(m, x - r, y + r)) return 1;
    if (map_wall_at(m, x + r, y + r)) return 1;
    return 0;
}

static void move_player(Game *g, float dx, float dy) {
    LevelMap *m = g->current_map;
    float nx = g->px + dx;
    float ny = g->py + dy;

    /* slide collision - try X then Y separately */
    if (!collision_at(m, nx, g->py))
        g->px = nx;
    if (!collision_at(m, g->px, ny))
        g->py = ny;
}

static void on_weapon_hit(float ox, float oy, float ray_angle, float dmg, void *ctx) {
    (void)ox; (void)oy;
    Game *g = (Game *)ctx;
    WeaponDef *w = &weapons[g->weapons.current];

    float rdx = cosf(ray_angle), rdy = sinf(ray_angle);
    float hit_dist;
    int hit_idx;
    if (enemy_raycast_hit(&g->enemies, g->px, g->py, rdx, rdy, w->range, &hit_dist, &hit_idx))
        enemy_damage(&g->enemies, hit_idx, dmg);
}

int game_init(Game *g, const char *asset_dir, int scale) {
    memset(g, 0, sizeof(*g));
    g_singleton = g;
    g->running = 1;
    g->health = MAX_HEALTH;

    map_load_all(asset_dir);
    texture_load_all(asset_dir);

    if (render_init(&g->render, scale) != 0) return -1;
    weapon_init(&g->weapons);
    input_init();
    game_load_level(g, 0);
    return 0;
}

void game_shutdown(Game *g) {
    render_shutdown(&g->render);
    g_singleton = NULL;
}

void game_load_level(Game *g, int idx) {
    if (idx < 0) idx = 0;
    if (idx >= NUM_LEVELS) idx = NUM_LEVELS - 1;
    g->level_idx = idx;
    g->current_map = &levels[idx];
    g->px = g->current_map->spawn_x;
    g->py = g->current_map->spawn_y;
    g->pa = g->current_map->spawn_angle;
    g->health = MAX_HEALTH;
    g->won = 0;
    g->level_clear_timer = 0.0f;
    enemy_init_group(&g->enemies, g->current_map);
    input_reset_mouse(&g->input);
}

void game_update(Game *g, float dt) {
    if (dt > 0.05f) dt = 0.05f;

    input_poll(&g->input);
    if (g->input.quit_req) { g->running = 0; return; }

    if (g->won == 2) return;

    if (g->won == 1) {
        g->level_clear_timer += dt;
        if (g->level_clear_timer > 2.0f) {
            if (g->level_idx + 1 < NUM_LEVELS)
                game_load_level(g, g->level_idx + 1);
            else
                g->won = 2;
        }
        goto draw;
    }

    /* mouse look */
    g->pa += g->input.mouse_dx;
    if (g->pa > 6.2831853f) g->pa -= 6.2831853f;
    if (g->pa < 0.0f) g->pa += 6.2831853f;

    /* weapon switch */
    if (g->input.weapon_key >= 0 && g->input.weapon_key < NUM_WEAPONS)
        weapon_switch(&g->weapons, g->input.weapon_key);

    /* movement */
    float speed = MOVE_SPEED * dt;
    if (g->input.run) speed *= RUN_MULT;

    float forward_x = cosf(g->pa), forward_y = sinf(g->pa);
    float right_x = cosf(g->pa + 1.5707963f), right_y = sinf(g->pa + 1.5707963f);

    float mx = 0.0f, my = 0.0f;
    if (g->input.forward)  { mx += forward_x * speed; my += forward_y * speed; }
    if (g->input.backward) { mx -= forward_x * speed; my -= forward_y * speed; }
    if (g->input.strafe_l) { mx -= right_x * speed * STRAFE_MULT; my -= right_y * speed * STRAFE_MULT; }
    if (g->input.strafe_r) { mx += right_x * speed * STRAFE_MULT; my += right_y * speed * STRAFE_MULT; }

    move_player(g, mx, my);

    /* weapon */
    g->weapons.cooldown -= dt;
    if (g->weapons.cooldown < 0.0f) g->weapons.cooldown = 0.0f;
    g->weapons.bob += dt * 8.0f;
    if (g->weapons.flash > 0.0f) g->weapons.flash -= dt;

    if (g->input.shoot)
        weapon_fire(&g->weapons, g->px, g->py, g->pa, on_weapon_hit, g);

    /* enemies */
    enemy_update(&g->enemies, dt, g->px, g->py, &g->health, g->current_map);

    if (g->health <= 0.0f) {
        game_load_level(g, g->level_idx);
    }

    /* level exit */
    float ex = g->px - g->current_map->exit_x;
    float ey = g->py - g->current_map->exit_y;
    if (ex * ex + ey * ey < 1.0f && enemy_alive_count(&g->enemies) == 0)
        g->won = 1;

    if (g->input.next_level) {
        g->input.next_level = 0;
        if (g->level_idx + 1 < NUM_LEVELS)
            game_load_level(g, g->level_idx + 1);
    }

draw:
    raycast_render(g->render.framebuffer, g->current_map, g->px, g->py, g->pa, &g->enemies);
    render_minimap(g->render.framebuffer, g->current_map, g->px, g->py, g->pa);
    render_hud(&g->render, g->health, g->weapons.current, &g->weapons,
               g->level_idx, enemy_alive_count(&g->enemies), g->won);
    render_present(&g->render);
}

void game_run(Game *g) {
    Uint64 prev = SDL_GetPerformanceCounter();
    double freq = (double)SDL_GetPerformanceFrequency();

    while (g->running) {
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)((now - prev) / freq);
        prev = now;
        game_update(g, dt);
        SDL_Delay(1);
    }
}
