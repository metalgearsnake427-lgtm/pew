#include "vidm_replay.h"

void vidm_replay_init(
    VidMReplayController *ctrl)
{
    if (!ctrl)
        return;

    ctrl->mode =
        VIDM_REPLAY_DISABLED;

    ctrl->pending_restart =
        false;

    ctrl->replay_count = 0;
}

void vidm_replay_set_mode(
    VidMReplayController *ctrl,
    VidMReplayMode mode)
{
    if (!ctrl)
        return;

    ctrl->mode = mode;
}

void vidm_replay_request(
    VidMReplayController *ctrl)
{
    if (!ctrl)
        return;

    ctrl->pending_restart = true;
}

bool vidm_replay_required(
    const VidMReplayController *ctrl)
{
    if (!ctrl)
        return false;

    return ctrl->pending_restart;
}

void vidm_replay_complete(
    VidMReplayController *ctrl)
{
    if (!ctrl)
        return;

    ctrl->pending_restart = false;
    ctrl->replay_count++;
}

uint64_t vidm_replay_count(
    const VidMReplayController *ctrl)
{
    if (!ctrl)
        return 0;

    return ctrl->replay_count;
}