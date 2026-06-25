#include "input.h"
#include "constants.h"

void input_init(void) {
    SDL_SetRelativeMouseMode(SDL_TRUE);
}

void input_reset_mouse(InputState *in) {
    in->mouse_dx = 0.0f;
}

void input_poll(InputState *in) {
    in->forward = in->backward = in->strafe_l = in->strafe_r = in->run = 0;
    in->shoot = 0;
    in->weapon_key = -1;
    in->mouse_dx = 0.0f;

    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    const Uint32 mouse = SDL_GetMouseState(NULL, NULL);
    in->forward  = keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP];
    in->backward = keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN];
    in->strafe_l = keys[SDL_SCANCODE_A];
    in->strafe_r = keys[SDL_SCANCODE_D];
    in->run      = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
    in->shoot    = (mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
            in->quit_req = 1;
            break;
        case SDL_KEYDOWN:
            if (ev.key.keysym.sym == SDLK_ESCAPE) in->quit_req = 1;
            if (ev.key.keysym.sym >= SDLK_1 && ev.key.keysym.sym <= SDLK_9)
                in->weapon_key = ev.key.keysym.sym - SDLK_1;
            if (ev.key.keysym.sym == SDLK_n) in->next_level = 1;
            break;
        case SDL_MOUSEMOTION:
            in->mouse_dx += ev.motion.xrel * MOUSE_SENS;
            break;
        }
    }
}
