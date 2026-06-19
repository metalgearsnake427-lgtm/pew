#include <stdio.h>
#include "vidm_pause.h"

int main(void)
{
    VidMPauseController player;

    vidm_pause_controller_init(&player);

    vidm_pause_controller_play(&player);

    printf("Playing: %d\n",
           vidm_pause_controller_is_playing(&player));

    vidm_pause_controller_pause(&player);

    printf("Paused: %d\n",
           vidm_pause_controller_is_paused(&player));

    vidm_pause_controller_resume(&player);

    printf("Playing: %d\n",
           vidm_pause_controller_is_playing(&player));

    vidm_pause_controller_stop(&player);

    return 0;
}