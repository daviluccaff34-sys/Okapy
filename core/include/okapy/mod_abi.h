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

typedef enum OkapyLogLevel {
    OKAPY_LOG_DEBUG = 0,
    OKAPY_LOG_INFO = 1,
    OKAPY_LOG_WARNING = 2,
    OKAPY_LOG_ERROR = 3
} OkapyLogLevel;

typedef enum OkapyConfigResult {
    OKAPY_CONFIG_OK = 0,
    OKAPY_CONFIG_NOT_FOUND = 1,
    OKAPY_CONFIG_BUFFER_TOO_SMALL = 2,
    OKAPY_CONFIG_ERROR = -1
} OkapyConfigResult;

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

    // Optional services. A null function pointer means that service is
    // unavailable. Strings passed to callbacks are UTF-8 and borrowed for
    // the duration of the call.
    void (*log)(void* host_context, int32_t level, const char* message);
    int32_t (*get_game_info)(
        void* host_context, OkapyGameInfoV1* out_info);

    // Config is namespaced by mod_id. config_get writes a NUL-terminated value
    // when capacity is sufficient and reports required_size including NUL.
    // It returns OK, NOT_FOUND, BUFFER_TOO_SMALL, or ERROR.
    int32_t (*config_get)(
        void* host_context, const char* mod_id, const char* key,
        char* buffer, uint32_t capacity, uint32_t* required_size);
    int32_t (*config_set)(
        void* host_context, const char* mod_id, const char* key,
        const char* value);
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

// The returned API and all pointed-to metadata must remain valid until the
// module is unloaded. The host validates struct sizes and required callbacks.
typedef const OkapyModApiV1* (*OkapyModQueryV1)(
    uint32_t requested_abi,
    const OkapyHostApiV1* host,
    OkapyModInfoV1* out_info);

OKAPY_MOD_EXPORT const OkapyModApiV1* okapy_mod_query(
    uint32_t requested_abi,
    const OkapyHostApiV1* host,
    OkapyModInfoV1* out_info);

#ifdef __cplusplus
}
#endif

#endif
