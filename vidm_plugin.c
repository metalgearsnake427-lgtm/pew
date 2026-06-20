#include "vidm_plugin.h"

#include <stdio.h>

static int
vidm_plugin_load(
    const VidMPlugin *plugin)
{
    if (!plugin)
        return -1;

    printf(
        "[VIDM] Loading Plugin: %s\n",
        plugin->info.name
    );

    if (plugin->initialize)
        return plugin->initialize();

    return 0;
}

static void
vidm_plugin_unload(
    const VidMPlugin *plugin)
{
    if (!plugin)
        return;

    if (plugin->shutdown)
        plugin->shutdown();
}