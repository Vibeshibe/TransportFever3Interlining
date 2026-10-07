#include <limits.h>
#include "native_candidate.h"

#define READ(call) do { tfx_status status_ = (call); if (status_ != TFX_OK) return status_; } while (0)
#define REJECT(why) do { result.reason = (why); *out = result; return TFX_OK; } while (0)

static int same_terminal(const tfx_line_stop *a, const tfx_line_stop *b) {
    return a->station_group == b->station_group && a->station_index == b->station_index &&
        a->station_entity == b->station_entity && a->terminal == b->terminal;
}

const char *it_reason_name(it_reason reason) {
    static const char *names[IT_REASON_COUNT] = {
        "candidate", "already_assigned", "same_line", "unsupported_leg",
        "vehicle_not_loading", "alternative_terminals", "boarding_mismatch",
        "alighting_mismatch", "actual_terminal_mismatch", "loading_disabled",
        "unloading_disabled", "unsupported_load_limit", "no_capacity",
        "no_platform_membership", "destination_missing", "not_in_loading_index",
        "load_timer_pending"
    };
    return reason >= 0 && reason < IT_REASON_COUNT ? names[reason] : "unknown";
}

tfx_status it_validate_candidate(const tfx_api_v5 *api, tfx_context context, int32_t person,
                                 int32_t vehicle, it_candidate *out) {
    if (!api || !out || person < 0 || vehicle < 0 ||
        api->v4.v3.v2.base.abi_version != TFX_TERMINAL_ABI_VERSION ||
        api->v4.v3.v2.base.struct_size != sizeof(*api) ||
        (api->v4.v3.v2.base.capabilities & 31u) != 31u ||
        !api->v4.v3.v2.base.read_observation || !api->v4.v3.v2.read_waiting_leg ||
        !api->v4.v3.v2.read_line_stop || !api->v4.v3.read_vehicle ||
        !api->v4.v3.read_stop_vehicles || !api->v4.read_person_platforms ||
        !api->read_terminal || !api->read_entity_presence) return TFX_INVALID_ARGUMENT;
    it_candidate result = {.requires_engine_revalidation = 1, .proposed_vehicle = vehicle,
        .proposed_line = -1, .boarding_stop = -1, .alighting_stop = -1,
        .boarding_station = -1, .boarding_terminal = -1, .alighting_station = -1, .alighting_terminal = -1};
    result.expected.struct_size = sizeof(result.expected);
    READ(api->v4.v3.v2.read_waiting_leg(context, person, &result.expected));
    const tfx_waiting_leg *leg = &result.expected;
    if (leg->vehicle != -1) REJECT(IT_ALREADY_ASSIGNED);
    if (leg->boarding_stop == INT32_MAX || leg->alighting_stop != leg->boarding_stop + 1)
        REJECT(IT_UNSUPPORTED_LEG);
    tfx_vehicle v = {.struct_size = sizeof(v)};
    READ(api->v4.v3.read_vehicle(context, vehicle, &v));
    result.proposed_line = v.line; result.boarding_stop = v.stop_index;
    if (leg->line == v.line) REJECT(IT_SAME_LINE);
    /* Pinned build: AT_TERMINAL=2; Loading=3. Arrived/Departing are not accepted. */
    if (v.state != 2 || v.load_state != 3 || v.user_stopped || v.line < 0 || v.stop_index < 0)
        REJECT(IT_VEHICLE_NOT_LOADING);
    if (v.stop_index == INT32_MAX) REJECT(IT_UNSUPPORTED_LEG);
    result.alighting_stop = v.stop_index + 1;
    tfx_line_stop old_board = {.struct_size = sizeof(old_board)}, old_alight = {.struct_size = sizeof(old_alight)};
    tfx_line_stop board = {.struct_size = sizeof(board)}, alight = {.struct_size = sizeof(alight)};
    READ(api->v4.v3.v2.read_line_stop(context, leg->line, leg->boarding_stop, &old_board));
    READ(api->v4.v3.v2.read_line_stop(context, leg->line, leg->alighting_stop, &old_alight));
    READ(api->v4.v3.v2.read_line_stop(context, v.line, v.stop_index, &board));
    if (result.alighting_stop >= board.stop_count) REJECT(IT_UNSUPPORTED_LEG); /* No loop wrap. */
    READ(api->v4.v3.v2.read_line_stop(context, v.line, result.alighting_stop, &alight));
    if (old_board.station_group == old_alight.station_group) REJECT(IT_UNSUPPORTED_LEG);
    if (old_board.has_alternative_terminals || old_alight.has_alternative_terminals ||
        board.has_alternative_terminals || alight.has_alternative_terminals) REJECT(IT_ALTERNATIVE_TERMINALS);
    if (!same_terminal(&old_board, &board)) REJECT(IT_BOARDING_MISMATCH);
    if (!same_terminal(&old_alight, &alight)) REJECT(IT_ALIGHTING_MISMATCH);
    result.boarding_station = board.station_entity; result.boarding_terminal = board.terminal;
    result.alighting_station = alight.station_entity; result.alighting_terminal = alight.terminal;
    /* Requiring the selected index to equal the configured index lets us compare
     * the already resolved entity without confusing group indices and entity IDs. */
    if (!v.arrival_locked || v.arrival_station_index != board.station_index || v.arrival_terminal != board.terminal)
        REJECT(IT_ACTUAL_TERMINAL_MISMATCH);
    if (!board.passenger_load_enabled || board.load_mode != 0) REJECT(IT_LOADING_DISABLED);
    /* Compartment allocation/rounding for fractional maxLoad is not established. */
    if (board.passenger_max_load_bits != 0x3f800000u) REJECT(IT_UNSUPPORTED_LOAD_LIMIT);
    uint64_t used = (uint64_t)v.passengers_onboard + v.passengers_reserved;
    if (used >= v.passenger_capacity || v.passenger_places_left != v.passenger_capacity - used)
        REJECT(IT_NO_CAPACITY);
    result.observed_places_left = v.passenger_places_left;
    uint32_t timer = v.next_passenger_load_seconds_bits;
    if ((timer & 0x7f800000u) == 0x7f800000u || (!(timer & 0x80000000u) && timer != 0))
        REJECT(IT_LOAD_TIMER_PENDING); /* Conservative: require ready now, not next delta. */
    tfx_person_platforms platforms = {.struct_size = sizeof(platforms)};
    READ(api->v4.read_person_platforms(context, person, &platforms));
    int member = 0;
    if (platforms.count > TFX_PERSON_PLATFORMS_SIZE) return TFX_INVALID_STATE;
    for (uint32_t i = 0; i < platforms.count; ++i)
        if (platforms.platforms[i].station_entity == board.station_entity && platforms.platforms[i].terminal == board.terminal)
            member = 1;
    if (!member) REJECT(IT_NO_PLATFORM_MEMBERSHIP);
    tfx_terminal boarding = {.struct_size = sizeof(boarding)}, alighting = {.struct_size = sizeof(alighting)};
    READ(api->read_terminal(context, board.station_entity, board.terminal, &boarding));
    READ(api->read_terminal(context, alight.station_entity, alight.terminal, &alighting));
    if (!boarding.passengers_load) REJECT(IT_LOADING_DISABLED);
    if (!alighting.passengers_unload) REJECT(IT_UNLOADING_DISABLED);
    if (leg->destination < 0) REJECT(IT_DESTINATION_MISSING);
    tfx_entity_presence destination = {.struct_size = sizeof(destination)};
    READ(api->read_entity_presence(context, leg->destination, &destination));
    if (!destination.has_components) REJECT(IT_DESTINATION_MISSING);
    tfx_stop_vehicles loading = {.struct_size = sizeof(loading)};
    READ(api->v4.v3.read_stop_vehicles(context, v.line, v.stop_index, &loading));
    if (loading.count > TFX_STOP_VEHICLES_SIZE) return TFX_INVALID_STATE;
    int indexed = 0;
    for (uint32_t i = 0; i < loading.count; ++i) if (loading.vehicles[i] == vehicle) indexed = 1;
    if (!indexed) REJECT(IT_NOT_IN_LOADING_INDEX);
    tfx_observation observation = {.struct_size = sizeof(observation)};
    READ(api->v4.v3.v2.base.read_observation(context, &observation));
    result.dispatch_sequence = observation.dispatch_sequence;
    result.reason = IT_CANDIDATE;
    *out = result;
    return TFX_OK;
}
