#include "vidm_pause.h"

void vidm_pause_controller_init(
    VidMPauseController *controller)
{
    if (!controller)
        return;

    controller->state =
        VIDM_PLAYBACK_STOPPED;
}

void vidm_pause_controller_play(
    VidMPauseController *controller)
{
    if (!controller)
        return;

    controller->state =
        VIDM_PLAYBACK_PLAYING;
}

void vidm_pause_controller_pause(
    VidMPauseController *controller)
{
    if (!controller)
        return;

    if (controller->state ==
        VIDM_PLAYBACK_PLAYING)
    {
        controller->state =
            VIDM_PLAYBACK_PAUSED;
    }
}

void vidm_pause_controller_resume(
    VidMPauseController *controller)
{
    if (!controller)
        return;

    if (controller->state ==
        VIDM_PLAYBACK_PAUSED)
    {
        controller->state =
            VIDM_PLAYBACK_PLAYING;
    }
}

void vidm_pause_controller_stop(
    VidMPauseController *controller)
{
    if (!controller)
        return;

    controller->state =
        VIDM_PLAYBACK_STOPPED;
}

void vidm_pause_controller_toggle(
    VidMPauseController *controller)
{
    if (!controller)
        return;

    switch (controller->state)
    {
        case VIDM_PLAYBACK_PLAYING:
            controller->state =
                VIDM_PLAYBACK_PAUSED;
            break;

        case VIDM_PLAYBACK_PAUSED:
            controller->state =
                VIDM_PLAYBACK_PLAYING;
            break;

        default:
            break;
    }
}

VidMPlaybackState vidm_pause_controller_state(
    const VidMPauseController *controller)
{
    if (!controller)
        return VIDM_PLAYBACK_STOPPED;

    return controller->state;
}

bool vidm_pause_controller_is_playing(
    const VidMPauseController *controller)
{
    if (!controller)
        return false;

    return controller->state ==
           VIDM_PLAYBACK_PLAYING;
}

bool vidm_pause_controller_is_paused(
    const VidMPauseController *controller)
{
    if (!controller)
        return false;

    return controller->state ==
           VIDM_PLAYBACK_PAUSED;
}