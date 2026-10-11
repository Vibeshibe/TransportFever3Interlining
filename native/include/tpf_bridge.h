#ifndef TPF_BRIDGE_H
#define TPF_BRIDGE_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TFX_ABI_VERSION 1u
#define TFX_QUERY_ABI_VERSION 2u
#define TFX_VEHICLE_ABI_VERSION 3u
#define TFX_PLATFORM_ABI_VERSION 4u
#define TFX_TERMINAL_ABI_VERSION 5u
#define TFX_CHANGE_ABI_VERSION 6u
#define TFX_LIFECYCLE_ABI_VERSION 7u
#define TFX_STOP_TERMINALS_ABI_VERSION 8u
#define TFX_LINE_SCAN_ABI_VERSION 9u
#define TFX_CAP_LINE_SCAN 512u
#define TFX_CAP_STOP_TERMINALS 256u
#define TFX_STOP_TERMINALS_SIZE 16u
#define TFX_CAP_PERSON_LIFECYCLE 128u
#define TFX_CAP_WAITING_LEG_CHANGE 32u
#define TFX_CAP_PERSON_JOURNEY 64u
#define TFX_CAP_TERMINAL_OBSERVATION 1u
#define TFX_CAP_PASSENGER_ROUTE_QUERIES 2u
#define TFX_CAP_VEHICLE_QUERIES 4u
#define TFX_CAP_PLATFORM_QUERIES 8u
#define TFX_CAP_TERMINAL_QUERIES 16u
#define TFX_PERSON_PLATFORMS_SIZE 16u
#define TFX_WAITING_PAGE_SIZE 16u
#define TFX_STOP_VEHICLES_SIZE 16u
#define TFX_HAS_TERMINAL_PLACE 1u
#define TFX_CLIENT_ID_SIZE 64u
#define TFX_MAX_CLIENTS 32u

typedef uint32_t tfx_status;
enum {
    TFX_OK = 0, TFX_INVALID_ARGUMENT, TFX_ABI_MISMATCH, TFX_DUPLICATE_CLIENT,
    TFX_BUSY, TFX_LIMIT_REACHED, TFX_NOT_FOUND, TFX_STALE_CONTEXT,
    TFX_WRONG_THREAD, TFX_UNSUPPORTED_BUILD, TFX_HOOK_CONFLICT,
    TFX_SYSTEM_ERROR, TFX_PROTECTION_ERROR, TFX_QUERY_UNAVAILABLE,
    TFX_INVALID_STATE, TFX_QUERY_BUDGET_EXHAUSTED,
    TFX_PENDING, TFX_REQUEST_CONFLICT, TFX_EXPECTED_STATE_CHANGED, TFX_POLICY_REJECTED
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

/* Observed native waiting sentinels. Their meanings are not established; only
 * vehicle == -1 is eligible for waiting-leg replacement. Never treat -2/-3 as IDs. */
#define TFX_WAITING_VEHICLE_SPECIAL_2 (-2)
#define TFX_WAITING_VEHICLE_SPECIAL_3 (-3)
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

/* Engine index of vehicles considered for loading at this line stop. A full
 * copy is required: more than 16 vehicles returns TFX_LIMIT_REACHED. */
typedef struct tfx_stop_vehicles {
    uint32_t struct_size, count;
    int32_t line, stop_index;
    int32_t vehicles[TFX_STOP_VEHICLES_SIZE];
} tfx_stop_vehicles;

/* Snapshot BEFORE terminal selection. Seats can be assigned by the original
 * Update after this callback. This is not a boarding eligibility decision. */
typedef struct tfx_vehicle {
    uint32_t struct_size;
    int32_t vehicle, line, stop_index;
    int32_t state, load_state; /* Build-specific enum values. */
    int32_t arrival_station_index, arrival_terminal; /* Index in station GROUP, not entity. */
    uint32_t arrival_locked, user_stopped;
    uint32_t passenger_capacity, passengers_onboard, passengers_reserved, passenger_places_left;
    uint32_t next_passenger_load_seconds_bits; /* Finite IEEE-754 float; may be negative. */
    uint32_t reserved;
} tfx_vehicle;

typedef struct tfx_api_v3 {
    tfx_api_v2 v2;
    tfx_status (*read_stop_vehicles)(tfx_context, int32_t line, int32_t stop, tfx_stop_vehicles *out);
    tfx_status (*read_vehicle)(tfx_context, int32_t vehicle, tfx_vehicle *out);
} tfx_api_v3;

typedef struct tfx_platform {
    int32_t station_entity, terminal; /* Station entity, unlike vehicle's station index. */
} tfx_platform;

/* Direct waiting-edge membership from this world's StationSystem. Empty means
 * no direct edge membership, NOT permission to board. Node fallback, terminal
 * loading flags and destination/unloading checks are separate requirements. */
typedef struct tfx_person_platforms {
    uint32_t struct_size, count;
    int32_t person, edge_entity, edge_index, place;
    tfx_platform platforms[TFX_PERSON_PLATFORMS_SIZE];
} tfx_person_platforms;

typedef struct tfx_api_v4 {
    tfx_api_v3 v3;
    tfx_status (*read_person_platforms)(tfx_context, int32_t person, tfx_person_platforms *out);
} tfx_api_v4;

typedef struct tfx_terminal {
    uint32_t struct_size;
    int32_t station_entity, terminal, terminal_count;
    uint32_t passengers_load, passengers_unload;
} tfx_terminal;

/* Presence means a nonempty, consistent component row/mask in this world.
 * It does not establish destination reachability or a stable entity generation. */
typedef struct tfx_entity_presence {
    uint32_t struct_size;
    int32_t entity;
    uint32_t has_components, reserved;
} tfx_entity_presence;

typedef struct tfx_api_v5 {
    tfx_api_v4 v4;
    tfx_status (*read_terminal)(tfx_context, int32_t station, int32_t terminal, tfx_terminal *out);
    tfx_status (*read_entity_presence)(tfx_context, int32_t entity, tfx_entity_presence *out);
} tfx_api_v5;

/* One callback-scoped proposal; the host stages it until all clients finish.
 * Revalidation gets a NEW token and must return TFX_OK to permit the operation.
 * Keep this function's module alive until dispatch returns. No engine pointers. */
typedef struct tfx_waiting_leg_change tfx_waiting_leg_change;
typedef tfx_status (*tfx_revalidate_change)(tfx_context, const tfx_waiting_leg_change *);
struct tfx_waiting_leg_change {
    uint32_t struct_size, reserved;
    uint64_t dispatch_sequence;
    tfx_waiting_leg expected;
    int32_t line, boarding_stop, alighting_stop, candidate_vehicle;
    tfx_revalidate_change revalidate;
};
typedef struct tfx_change_result {
    uint32_t struct_size;
    tfx_status status; /* PENDING is staging only; OK means replacement, not boarding. */
    uint64_t ticket, dispatch_sequence;
    int32_t person;
    uint32_t engine_attempted;
} tfx_change_result;
#define TFX_JOURNEY_WAITING 1u
#define TFX_JOURNEY_ONBOARD 2u
#define TFX_JOURNEY_MOVING 4u
#define TFX_JOURNEY_AT_BUILDING 8u
typedef struct tfx_person_journey {
    uint32_t struct_size, flags;
    uint32_t world_id, reserved; /* Opaque process-local ordinal, not an engine pointer. */
    int32_t person, destination, line, boarding_stop, alighting_stop, vehicle;
} tfx_person_journey;
typedef struct tfx_api_v6 {
    tfx_api_v5 v5;
    tfx_status (*request_waiting_leg_change)(tfx_context, const tfx_waiting_leg_change *, uint64_t *ticket);
    tfx_status (*read_change_result)(tfx_context, uint64_t ticket, tfx_change_result *out);
    tfx_status (*read_person_journey)(tfx_context, int32_t person, tfx_person_journey *out);
} tfx_api_v6;

/* Current component identity only: no stable entity generation or removal cause.
 * A valid absent SimPerson is TFX_OK with flags cleared, not a query failure.
 * Destination slots are residence, shopping, work; -1 means unassigned. */
#define TFX_LIFECYCLE_ENTITY_PRESENT 1u
#define TFX_LIFECYCLE_PERSON_PRESENT 2u
typedef struct tfx_person_lifecycle {
    uint32_t struct_size, world_id, flags;
    int32_t person, target, destinations[3];
} tfx_person_lifecycle;
typedef struct tfx_api_v7 {
    tfx_api_v6 v6;
    tfx_status (*read_person_lifecycle)(tfx_context, int32_t person, tfx_person_lifecycle *out);
} tfx_api_v7;

/* Complete copied primary + alternatives, in configuration order. Entry zero
 * is primary. Station indices belong to stop.station_group; entities are resolved
 * in this callback's world. Oversize results fail, never silently truncate. */
typedef struct tfx_stop_terminal {
    int32_t station_index, station_entity, terminal;
} tfx_stop_terminal;
typedef struct tfx_stop_terminals {
    uint32_t struct_size, count;
    tfx_line_stop stop;
    tfx_stop_terminal terminals[TFX_STOP_TERMINALS_SIZE];
} tfx_stop_terminals;
typedef struct tfx_api_v8 {
    tfx_api_v7 v7;
    tfx_status (*read_stop_terminals)(tfx_context, int32_t line, int32_t stop, tfx_stop_terminals *out);
} tfx_api_v8;

/* Paged line-component IDs, including services without waiting passengers.
 * Results are discovery hints; every stop/vehicle is subsequently read fresh. */
typedef struct tfx_line_page {
    uint32_t struct_size, count;
    int32_t next_entity;
    uint32_t reserved;
    int32_t lines[TFX_WAITING_PAGE_SIZE];
} tfx_line_page;
typedef struct tfx_api_v9 {
    tfx_api_v8 v8;
    tfx_status (*scan_lines)(tfx_context, int32_t first_entity, tfx_line_page *out);
} tfx_api_v9;

/* struct_size must match the requested API version. No engine pointers cross this ABI. */
__declspec(dllexport) tfx_status tfx_get_api(uint32_t version, tfx_api *out);
#ifdef __cplusplus
}
#endif
#endif
