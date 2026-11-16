#ifndef OKAPY_MOD_ABI_H
#define OKAPY_MOD_ABI_H

#include <stdint.h>

#if defined(_WIN32)
#  if defined(OKAPY_MOD_BUILD)
#    define OKAPY_MOD_EXPORT __declspec(dllexport)
#  else
#    define OKAPY_MOD_EXPORT __declspec(dllimport)
#  endif
#else
#  define OKAPY_MOD_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define OKAPY_MOD_ABI_VERSION_1 1u
#define OKAPY_MOD_QUERY_SYMBOL "okapy_mod_query"

typedef enum OkapyLogLevel { OKAPY_LOG_DEBUG=0, OKAPY_LOG_INFO=1, OKAPY_LOG_WARNING=2, OKAPY_LOG_ERROR=3 } OkapyLogLevel;
typedef enum OkapyConfigResult { OKAPY_CONFIG_OK=0, OKAPY_CONFIG_NOT_FOUND=1, OKAPY_CONFIG_BUFFER_TOO_SMALL=2, OKAPY_CONFIG_PERMISSION_DENIED=3, OKAPY_CONFIG_ERROR=-1 } OkapyConfigResult;
typedef enum OkapyEventResult { OKAPY_EVENT_OK=0, OKAPY_EVENT_INVALID_ARGUMENT=1, OKAPY_EVENT_NOT_FOUND=2, OKAPY_EVENT_PERMISSION_DENIED=3, OKAPY_EVENT_ERROR=-1 } OkapyEventResult;

typedef void (*OkapyEventCallbackV1)(void* user_data, const char* event_name, const char* payload);

typedef struct OkapyGameInfoV1 {
    uint32_t struct_size;
    uint32_t abi_version;
    const char* game_id;
    const char* game_version;
    const char* platform;
    const char* architecture;
} OkapyGameInfoV1;

typedef struct OkapyHostApiV1 {
    uint32_t struct_size;
    uint32_t abi_version;
    void* host_context;
    void (*log)(void* host_context, int32_t level, const char* message);
    int32_t (*get_game_info)(void* host_context, OkapyGameInfoV1* out_info);
    int32_t (*config_get)(void* host_context, const char* mod_id, const char* key, char* buffer, uint32_t capacity, uint32_t* required_size);
    int32_t (*config_set)(void* host_context, const char* mod_id, const char* key, const char* value);
    int32_t (*event_subscribe)(void* host_context, const char* mod_id, const char* event_name, OkapyEventCallbackV1 callback, void* user_data, uint64_t* out_token);
    int32_t (*event_unsubscribe)(void* host_context, const char* mod_id, uint64_t token);
    int32_t (*event_publish)(void* host_context, const char* mod_id, const char* event_name, const char* payload, uint32_t* out_listener_count);
    /* 1 = granted, 0 = denied, -1 = invalid input. */
    int32_t (*permission_check)(void* host_context, const char* mod_id, const char* permission);
} OkapyHostApiV1;

typedef struct OkapyModInfoV1 {
    uint32_t struct_size;
    uint32_t abi_version;
    const char* id;
    const char* name;
    const char* version;
} OkapyModInfoV1;

typedef struct OkapyModApiV1 {
    uint32_t struct_size;
    uint32_t abi_version;
    void* mod_context;
    int32_t (*initialize)(void* mod_context, const OkapyHostApiV1* host);
    void (*shutdown)(void* mod_context);
} OkapyModApiV1;

typedef const OkapyModApiV1* (*OkapyModQueryV1)(uint32_t requested_abi, const OkapyHostApiV1* host, OkapyModInfoV1* out_info);

OKAPY_MOD_EXPORT const OkapyModApiV1* okapy_mod_query(uint32_t requested_abi, const OkapyHostApiV1* host, OkapyModInfoV1* out_info);

#ifdef __cplusplus
}
#endif
#endif
