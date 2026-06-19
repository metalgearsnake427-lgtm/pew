#ifndef VIDM_PAUSE_H
#define VIDM_PAUSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef enum
{
    VIDM_PLAYBACK_STOPPED = 0,
    VIDM_PLAYBACK_PLAYING,
    VIDM_PLAYBACK_PAUSED
} VidMPlaybackState;

typedef struct VidMPauseController
{
    VidMPlaybackState state;
} VidMPauseController;

/* Lifecycle */
void vidm_pause_controller_init(
    VidMPauseController *controller
);

/* Playback Control */
void vidm_pause_controller_play(
    VidMPauseController *controller
);

void vidm_pause_controller_pause(
    VidMPauseController *controller
);

void vidm_pause_controller_resume(
    VidMPauseController *controller
);

void vidm_pause_controller_stop(
    VidMPauseController *controller
);

void vidm_pause_controller_toggle(
    VidMPauseController *controller
);

/* State Queries */
VidMPlaybackState vidm_pause_controller_state(
    const VidMPauseController *controller
);

bool vidm_pause_controller_is_playing(
    const VidMPauseController *controller
);

bool vidm_pause_controller_is_paused(
    const VidMPauseController *controller
);

#ifdef __cplusplus
}
#endif

#endif