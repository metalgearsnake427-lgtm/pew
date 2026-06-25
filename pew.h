#ifndef PEW_ADVANCED_H
#define PEW_ADVANCED_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PEW_VERSION_MAJOR 1
#define PEW_VERSION_MINOR 0
#define PEW_VERSION_PATCH 0

#define PEW_API_VERSION 100

typedef enum
{
    PEW_OK = 0,

    PEW_ERR_UNKNOWN = -1,
    PEW_ERR_SYNTAX = -2,
    PEW_ERR_RUNTIME = -3,
    PEW_ERR_MEMORY = -4,
    PEW_ERR_PLUGIN = -5,
    PEW_ERR_IO = -6,
    PEW_ERR_INVALID_ARG = -7

} PewResult;


typedef enum
{
    PEW_FORMAT_FIXED,
    PEW_FORMAT_SCIENTIFIC,
    PEW_FORMAT_AUTO

} PewFormat;
typedef struct
{
    uint8_t interactive;
    uint8_t verbose_steps;

    uint32_t precision;

    PewFormat format;

} PewConfig;

typedef struct PewContext PewContext;
typedef void* (*PewMallocFn)(size_t);
typedef void  (*PewFreeFn)(void*);

typedef struct
{
    PewMallocFn malloc_fn;
    PewFreeFn free_fn;

} PewAllocator;

PewContext*
pew_create(
    const PewConfig* config,
    const PewAllocator* allocator
);

void
pew_destroy(
    PewContext* ctx
);

PewResult
pew_execute(
    PewContext* ctx,
    const char* source,
    char* output,
    size_t output_size
);

const char*
pew_last_error(
    PewContext* ctx
);

typedef struct
{
    const char* name;
    const char* author;
    const char* version;

    uint32_t api_version;

    PewResult (*init)(PewContext* ctx);

    PewResult (*handle_command)(
        PewContext* ctx,
        const char* cmd,
        const char* args
    );

    void (*shutdown)(
        PewContext* ctx
    );

} PewPlugin;

PewResult
pew_load_plugin(
    PewContext* ctx,
    const PewPlugin* plugin
);

PewResult
pew_unload_plugin(
    PewContext* ctx,
    const char* plugin_name
);

typedef enum
{
    PEW_LOG_DEBUG,
    PEW_LOG_INFO,
    PEW_LOG_WARNING,
    PEW_LOG_ERROR

} PewLogLevel;

typedef void (*PewLogCallback)(
    PewLogLevel level,
    const char* message
);

void
pew_set_logger(
    PewContext* ctx,
    PewLogCallback callback
);

void
pew_shutdown_all(void);

#ifdef __cplusplus
}
#endif

#endif /
