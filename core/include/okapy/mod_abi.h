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

typedef struct OkapyHostApiV1 {
    uint32_t struct_size;
    uint32_t abi_version;
    void* host_context;
    void (*log)(void* host_context, int32_t level, const char* message);
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
