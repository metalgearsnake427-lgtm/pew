#ifndef VIDM_MEDIA_ENGINE_H
#define VIDM_MEDIA_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct
{
    bool auto_download_decoders;
    bool allow_experimental_formats;
    bool enable_plugin_loading;
    bool sandbox_plugins;
} VidMEngineConfig;

int vidm_engine_init(
    const VidMEngineConfig *config);

int vidm_engine_open(
    const char *filepath);

int vidm_engine_play(void);

int vidm_engine_pause(void);

int vidm_engine_stop(void);

void vidm_engine_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif