#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>

typedef struct {
    int forward, backward, strafe_l, strafe_r, run;
    int shoot;
    int quit_req;
    int next_level;
    int weapon_key;
    float mouse_dx;
} InputState;

void input_init(void);
void input_poll(InputState *in);
void input_reset_mouse(InputState *in);

#endif
