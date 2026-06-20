#ifndef VIDM_AUDIO_ENGINE_H
#define VIDM_AUDIO_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define VIDM_AUDIO_VERSION_MAJOR 1
#define VIDM_AUDIO_VERSION_MINOR 0
#define VIDM_AUDIO_VERSION_PATCH 0

#define VIDM_MAX_AUDIO_OBJECTS 512
#define VIDM_MAX_CHANNELS 64

typedef enum
{
    VIDM_AUDIO_OK = 0,
    VIDM_AUDIO_ERROR = -1,
    VIDM_AUDIO_INVALID_ARGUMENT = -2,
    VIDM_AUDIO_OUT_OF_MEMORY = -3,
    VIDM_AUDIO_DEVICE_FAILURE = -4
} VidMAudioResult;

typedef enum
{
    VIDM_LAYOUT_MONO = 1,
    VIDM_LAYOUT_STEREO = 2,
    VIDM_LAYOUT_5_1 = 6,
    VIDM_LAYOUT_7_1 = 8
} VidMAudioLayout;

typedef struct
{
    float x;
    float y;
    float z;
} VidMVector3;

typedef struct
{
    uint32_t id;

    VidMVector3 position;
    VidMVector3 velocity;

    float gain;
    float pitch;

    bool active;
} VidMAudioObject;

typedef struct
{
    uint32_t sample_rate;
    uint32_t buffer_size;

    VidMAudioLayout layout;

    bool enable_spatial_audio;
    bool enable_hrtf;
    bool enable_reverb;
    bool enable_dynamic_range_control;
} VidMAudioConfig;

typedef struct VidMAudioEngine VidMAudioEngine;

/* Lifecycle */

VidMAudioEngine *
vidm_audio_create(
    const VidMAudioConfig *config
);

void
vidm_audio_destroy(
    VidMAudioEngine *engine
);

/* Playback */

VidMAudioResult
vidm_audio_start(
    VidMAudioEngine *engine
);

VidMAudioResult
vidm_audio_stop(
    VidMAudioEngine *engine
);

VidMAudioResult
vidm_audio_pause(
    VidMAudioEngine *engine
);

VidMAudioResult
vidm_audio_resume(
    VidMAudioEngine *engine
);

/* Audio Objects */

uint32_t
vidm_audio_object_create(
    VidMAudioEngine *engine
);

VidMAudioResult
vidm_audio_object_destroy(
    VidMAudioEngine *engine,
    uint32_t object_id
);

VidMAudioResult
vidm_audio_object_set_position(
    VidMAudioEngine *engine,
    uint32_t object_id,
    float x,
    float y,
    float z
);

VidMAudioResult
vidm_audio_object_set_gain(
    VidMAudioEngine *engine,
    uint32_t object_id,
    float gain
);

/* Listener */

VidMAudioResult
vidm_audio_listener_set_position(
    VidMAudioEngine *engine,
    float x,
    float y,
    float z
);

/* DSP */

VidMAudioResult
vidm_audio_enable_hrtf(
    VidMAudioEngine *engine,
    bool enable
);

VidMAudioResult
vidm_audio_enable_reverb(
    VidMAudioEngine *engine,
    bool enable
);

VidMAudioResult
vidm_audio_enable_spatial_audio(
    VidMAudioEngine *engine,
    bool enable
);

#ifdef __cplusplus
}
#endif

#endif /* VIDM_AUDIO_ENGINE_H */