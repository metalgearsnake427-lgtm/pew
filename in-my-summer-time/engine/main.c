#include "game.h"
#include <stdio.h>
#include <string.h>

static const char *find_assets(int argc, char **argv) {
    static const char *candidates[] = {
        "assets",
        "../assets",
        "in-my-summer-time/assets",
        NULL
    };
    if (argc > 1) return argv[1];
    for (int i = 0; candidates[i]; i++) {
        char path[512];
        snprintf(path, sizeof(path), "%s/levels/level_1.map", candidates[i]);
        FILE *f = fopen(path, "r");
        if (f) { fclose(f); return candidates[i]; }
    }
    return "assets";
}

int main(int argc, char **argv) {
    const char *assets = find_assets(argc, argv);
    int scale = 3;
    if (argc > 2) scale = atoi(argv[2]);
    if (scale < 1) scale = 1;
    if (scale > 6) scale = 6;

    Game game;
    if (game_init(&game, assets, scale) != 0) {
        fprintf(stderr, "Failed to init game (SDL2 required). Assets: %s\n", assets);
        return 1;
    }

    printf("in my summer time, all alone\n");
    printf("WASD move | Mouse look | LMB shoot | 1-9 weapons | Shift run | Esc quit\n");
    printf("Clear all enemies and reach the exit portal to advance.\n");

    game_run(&game);
    game_shutdown(&game);
    return 0;
}
