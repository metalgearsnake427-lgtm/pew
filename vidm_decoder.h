#ifndef VIDM_DECODER_H
#define VIDM_DECODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define VIDM_MAX_EXTENSION_LENGTH 32
#define VIDM_MAX_PLUGIN_NAME      128
#define VIDM_MAX_URL              512

typedef enum
{
    VIDM_DECODER_NATIVE = 0,
    VIDM_DECODER_PLUGIN,
    VIDM_DECODER_REMOTE
} VidMDecoderType;

typedef struct
{
    char extension[VIDM_MAX_EXTENSION_LENGTH];
    char decoder_name[VIDM_MAX_PLUGIN_NAME];
    VidMDecoderType type;
    bool available;
} VidMDecoderInfo;

typedef struct
{
    int (*open)(const char *path);
    int (*decode)(void *frame);
    int (*seek)(double seconds);
    int (*close)(void);
} VidMDecoderAPI;

typedef struct
{
    VidMDecoderInfo info;
    VidMDecoderAPI api;
    void *handle;
} VidMDecoder;

int vidm_decoder_registry_init(void);

int vidm_decoder_register(
    const VidMDecoder *decoder);

const VidMDecoder *
vidm_decoder_find(
    const char *extension);

bool vidm_decoder_exists(
    const char *extension);

int vidm_decoder_download(
    const char *extension);

void vidm_decoder_registry_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif