#ifndef VIDM_PLUGIN_H
#define VIDM_PLUGIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define VIDM_PLUGIN_API_VERSION 1

typedef enum
{
    VIDM_PLUGIN_AUDIO,
    VIDM_PLUGIN_VIDEO,
    VIDM_PLUGIN_VISUALIZER,
    VIDM_PLUGIN_THEME,
    VIDM_PLUGIN_SCRIPT,
    VIDM_PLUGIN_CUSTOM
}
VidMPluginType;

typedef struct
{
    const char *name;
    const char *author;
    const char *version;

    VidMPluginType type;
}
VidMPluginInfo;

typedef struct
{
    VidMPluginInfo info;

    int  (*initialize)(void);
    void (*shutdown)(void);
}
VidMPlugin;

const VidMPlugin*
vidm_plugin_entry(void);

#ifdef __cplusplus
}
#endif

#endif
