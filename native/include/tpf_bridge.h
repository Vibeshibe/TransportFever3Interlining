#ifndef TPF_BRIDGE_H
#define TPF_BRIDGE_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TFX_ABI_VERSION 1u
#define TFX_QUERY_ABI_VERSION 2u
#define TFX_CAP_TERMINAL_OBSERVATION 1u
#define TFX_CAP_PASSENGER_ROUTE_QUERIES 2u
#define TFX_WAITING_PAGE_SIZE 16u
#define TFX_HAS_TERMINAL_PLACE 1u
#define TFX_CLIENT_ID_SIZE 64u
#define TFX_MAX_CLIENTS 32u

typedef uint32_t tfx_status;
enum {
    TFX_OK = 0, TFX_INVALID_ARGUMENT, TFX_ABI_MISMATCH, TFX_DUPLICATE_CLIENT,
    TFX_BUSY, TFX_LIMIT_REACHED, TFX_NOT_FOUND, TFX_STALE_CONTEXT,
    TFX_WRONG_THREAD, TFX_UNSUPPORTED_BUILD, TFX_HOOK_CONFLICT,
    TFX_SYSTEM_ERROR, TFX_PROTECTION_ERROR, TFX_QUERY_UNAVAILABLE,
    TFX_INVALID_STATE, TFX_QUERY_BUDGET_EXHAUSTED
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

typedef struct tfx_waiting_leg {
    uint32_t struct_size;
    int32_t person, line, boarding_stop, alighting_stop, vehicle, destination;
    uint32_t flags;
    uint64_t arrival_time_bits; /* Opaque original 8 bytes; no time conversion. */
    uint8_t terminal_place_bytes[12]; /* Opaque SimPersonAtTerminal value, if flagged. */
    uint32_t reserved;
} tfx_waiting_leg;

typedef struct tfx_line_stop {
    uint32_t struct_size;
    int32_t line, stop_index, stop_count, station_group, station_index;
    int32_t station_entity, terminal, load_mode;
    uint32_t has_alternative_terminals, passenger_load_enabled;
    uint32_t passenger_max_load_bits; /* IEEE-754 float, fraction [0,1]. */
} tfx_line_stop;

typedef struct tfx_waiting_page {
    uint32_t struct_size, count;
    int32_t next_entity; /* -1 at end. Cursor is NOT a stable snapshot across calls. */
    uint32_t reserved;
    int32_t persons[TFX_WAITING_PAGE_SIZE];
} tfx_waiting_page;

/* ABI 1 remains binary compatible. Request version 2 with &api.base and the
 * full v2 struct_size. Queries only work within the caller's live callback. */
typedef struct tfx_api_v2 {
    tfx_api base;
    tfx_status (*read_waiting_leg)(tfx_context, int32_t person, tfx_waiting_leg *out);
    tfx_status (*read_line_stop)(tfx_context, int32_t line, int32_t stop, tfx_line_stop *out);
    tfx_status (*scan_waiting_people)(tfx_context, int32_t first_entity, tfx_waiting_page *out);
} tfx_api_v2;

/* struct_size must match the requested API version. No engine pointers cross this ABI. */
__declspec(dllexport) tfx_status tfx_get_api(uint32_t version, tfx_api *out);
#ifdef __cplusplus
}
#endif
#endif
