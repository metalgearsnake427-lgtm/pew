#include "vidm_plugin.h"

#include <stdio.h>

static int
hello_init(void)
{
    printf(
        "Hello Plugin Initialized\n"
    );

    return 0;
}

static void
hello_shutdown(void)
{
    printf(
        "Hello Plugin Shutdown\n"
    );
}

static const VidMPlugin plugin =
{
    .info =
    {
        .name = "Hello Plugin",
        .author = "Sayan",
        .version = "1.0.0",
        .type = VIDM_PLUGIN_CUSTOM
    },

    .initialize = hello_init,
    .shutdown = hello_shutdown
};

const VidMPlugin*
vidm_plugin_entry(void)
{
    return &plugin;
}