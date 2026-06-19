#ifndef VIDM_REPLAY_H
#define VIDM_REPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    VIDM_REPLAY_DISABLED = 0,
    VIDM_REPLAY_SINGLE,
    VIDM_REPLAY_LOOP
} VidMReplayMode;

typedef struct
{
    VidMReplayMode mode;
    bool pending_restart;
    uint64_t replay_count;
} VidMReplayController;

/* Lifecycle */
void vidm_replay_init(
    VidMReplayController *ctrl
);

/* Configuration */
void vidm_replay_set_mode(
    VidMReplayController *ctrl,
    VidMReplayMode mode
);

/* State */
void vidm_replay_request(
    VidMReplayController *ctrl
);

bool vidm_replay_required(
    const VidMReplayController *ctrl
);

void vidm_replay_complete(
    VidMReplayController *ctrl
);

/* Statistics */
uint64_t vidm_replay_count(
    const VidMReplayController *ctrl
);

#ifdef __cplusplus
}
#endif

#endif
