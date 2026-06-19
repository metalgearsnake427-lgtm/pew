#ifndef VIDM_PAUSE_H
#define VIDM_PAUSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef enum
{
    VIDM_STATE_STOPPED = 0,
    VIDM_STATE_PLAYING,
    VIDM_STATE_PAUSED,
    VIDM_STATE_BUFFERING,
    VIDM_STATE_SEEKING
} VidMState;

typedef struct
{
    VidMState state;
} VidMPauseController;

/* Initialization */
void vidm_pause_init(VidMPauseController *ctrl);

/* State Changes */
void vidm_play(VidMPauseController *ctrl);
void vidm_pause(VidMPauseController *ctrl);
void vidm_resume(VidMPauseController *ctrl);
void vidm_stop(VidMPauseController *ctrl);
void vidm_toggle(VidMPauseController *ctrl);

/* Queries */
bool vidm_is_playing(const VidMPauseController *ctrl);
bool vidm_is_paused(const VidMPauseController *ctrl);
bool vidm_is_stopped(const VidMPauseController *ctrl);

VidMState vidm_get_state(const VidMPauseController *ctrl);

#ifdef __cplusplus
}
#endif

#endif