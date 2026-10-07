#ifndef TPF_BRIDGE_H
#define TPF_BRIDGE_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TFX_ABI_VERSION 1u
#define TFX_CAP_TERMINAL_OBSERVATION 1u
#define TFX_CLIENT_ID_SIZE 64u
#define TFX_MAX_CLIENTS 32u

typedef uint32_t tfx_status;
enum {
    TFX_OK = 0, TFX_INVALID_ARGUMENT, TFX_ABI_MISMATCH, TFX_DUPLICATE_CLIENT,
    TFX_BUSY, TFX_LIMIT_REACHED, TFX_NOT_FOUND, TFX_STALE_CONTEXT,
    TFX_WRONG_THREAD, TFX_UNSUPPORTED_BUILD, TFX_HOOK_CONFLICT,
    TFX_SYSTEM_ERROR, TFX_PROTECTION_ERROR
};

/* Tokens expire when this particular client's callback returns. */
typedef uint64_t tfx_context;
typedef struct tfx_observation {
    uint32_t struct_size;
    uint32_t delta_seconds_bits; /* Exact IEEE-754 bits of incoming XMM3 float. */
    uint64_t dispatch_sequence; /* Invocation count, NOT a game tick/world ID. */
} tfx_observation;

/* Callbacks must return normally, promptly, and without unloading their module. */
typedef void (*tfx_before_terminal_selection)(tfx_context, void *user_data);
typedef struct tfx_client {
    uint32_t struct_size;
    uint32_t reserved; /* Must be zero. */
    char id[TFX_CLIENT_ID_SIZE]; /* ASCII [a-z0-9_.-], nonempty, NUL terminated. */
    tfx_before_terminal_selection before_terminal_selection;
    void *user_data;
} tfx_client;

typedef struct tfx_api {
    uint32_t struct_size;
    uint32_t abi_version;
    uint64_t capabilities; /* No passenger mutation capability in version 1. */
    tfx_status (*register_client)(const tfx_client *client);
    tfx_status (*unregister_client)(const char *id);
    tfx_status (*read_observation)(tfx_context context, tfx_observation *out);
} tfx_api;

/* out->struct_size must equal sizeof(tfx_api). No engine pointers cross this ABI. */
__declspec(dllexport) tfx_status tfx_get_api(uint32_t version, tfx_api *out);
#ifdef __cplusplus
}
#endif
#endif
