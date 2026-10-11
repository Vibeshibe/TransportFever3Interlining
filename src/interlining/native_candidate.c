#include "native_candidate.h"

#define READ(call) do { tfx_status status_ = (call); if (status_ != TFX_OK) return status_; } while (0)
#define REJECT(why) do { result.reason = (why); *out = result; return TFX_OK; } while (0)

static int same_station(const tfx_line_stop *a, const tfx_line_stop *b) {
    return a->station_group == b->station_group && a->station_index == b->station_index &&
        a->station_entity == b->station_entity;
}

const char *it_reason_name(it_reason reason) {
    static const char *names[IT_REASON_COUNT] = {
        "candidate", "already_assigned", "same_line", "unsupported_leg",
        "vehicle_not_loading", "alternative_terminals", "boarding_mismatch",
        "alighting_mismatch", "actual_terminal_mismatch", "loading_disabled",
        "unloading_disabled", "unsupported_load_limit", "no_capacity",
        "no_platform_membership", "destination_missing", "not_in_loading_index",
        "load_timer_pending", "route_divergence", "unsupported_waiting_state", "terminal_limit"
    };
    return reason >= 0 && reason < IT_REASON_COUNT ? names[reason] : "unknown";
}

tfx_status it_validate_candidate(const tfx_api_v8 *bridge, tfx_context context, int32_t person,
                                 int32_t vehicle, it_candidate *out) {
    if (!bridge) return TFX_INVALID_ARGUMENT;
    const tfx_api_v5 *api = &bridge->v7.v6.v5;
    if (!out || person < 0 || vehicle < 0 ||
        api->v4.v3.v2.base.abi_version != TFX_STOP_TERMINALS_ABI_VERSION ||
        api->v4.v3.v2.base.struct_size != sizeof(*bridge) ||
        (api->v4.v3.v2.base.capabilities & 287u) != 287u ||
        !api->v4.v3.v2.base.read_observation || !api->v4.v3.v2.read_waiting_leg ||
        !api->v4.v3.v2.read_line_stop || !api->v4.v3.read_vehicle ||
        !api->v4.v3.read_stop_vehicles || !api->v4.read_person_platforms ||
        !api->read_terminal || !api->read_entity_presence || !bridge->read_stop_terminals) return TFX_INVALID_ARGUMENT;
    it_candidate result = {.requires_engine_revalidation = 1, .proposed_vehicle = vehicle,
        .proposed_line = -1, .boarding_stop = -1, .alighting_stop = -1,
        .boarding_station = -1, .boarding_terminal = -1, .alighting_station = -1, .alighting_terminal = -1};
    result.expected.struct_size = sizeof(result.expected);
    READ(api->v4.v3.v2.read_waiting_leg(context, person, &result.expected));
    const tfx_waiting_leg *leg = &result.expected;
    if (leg->vehicle < -1) REJECT(IT_UNSUPPORTED_WAITING_STATE);
    if (leg->vehicle != -1) REJECT(IT_ALREADY_ASSIGNED);
    if (leg->line < 0 || leg->boarding_stop < 0 || leg->alighting_stop < 0 ||
        leg->alighting_stop == leg->boarding_stop)
        REJECT(IT_UNSUPPORTED_LEG);
    tfx_vehicle v = {.struct_size = sizeof(v)};
    READ(api->v4.v3.read_vehicle(context, vehicle, &v));
    result.proposed_line = v.line; result.boarding_stop = v.stop_index;
    if (leg->line == v.line) REJECT(IT_SAME_LINE);
    /* Pinned build: AT_TERMINAL=2; Loading=3. Arrived/Departing are not accepted. */
    if (v.state != 2 || v.load_state != 3 || v.user_stopped || v.line < 0 || v.stop_index < 0)
        REJECT(IT_VEHICLE_NOT_LOADING);
    tfx_line_stop original[IT_MAX_LEG_HOPS + 1];
    int32_t hops = 0;
    for (int32_t i = 0; i <= hops; ++i) {
        int32_t index = i ? (int32_t)(((int64_t)leg->boarding_stop + i) % original[0].stop_count) : leg->boarding_stop;
        original[i] = (tfx_line_stop){.struct_size = sizeof(original[i])};
        READ(api->v4.v3.v2.read_line_stop(context, leg->line, index, &original[i]));
        if (original[i].line != leg->line || original[i].stop_index != index ||
            original[i].station_group < 0 ||
            original[i].stop_count != original[0].stop_count) return TFX_INVALID_STATE;
        if (i == 0) {
            int32_t count = original[0].stop_count;
            if (count <= leg->boarding_stop || count <= leg->alighting_stop) REJECT(IT_UNSUPPORTED_LEG);
            /* A native line list is cyclic, including a shuttle's return to its
             * first terminus. Preserve native indices; bound forward distance. */
            int64_t distance = (int64_t)leg->alighting_stop - leg->boarding_stop;
            if (distance < 0) distance += count;
            if (distance < 1 || distance > IT_MAX_LEG_HOPS) REJECT(IT_UNSUPPORTED_LEG);
            hops = (int32_t)distance;
        }
        result.route.original[result.route.original_count++] = original[i];
        /* Repeated stations make a turn/loop ambiguous. Never shorten that leg. */
        for (int32_t j = 0; j < i; ++j)
            if (original[j].station_group == original[i].station_group) REJECT(IT_UNSUPPORTED_LEG);
    }
    tfx_line_stop old_board = original[0], old_alight = original[hops];
    tfx_line_stop board = {.struct_size = sizeof(board)}, alight = {.struct_size = sizeof(alight)};
    result.route.boarding_options.struct_size = sizeof(result.route.boarding_options);
    tfx_status options_status = bridge->read_stop_terminals(context, v.line, v.stop_index, &result.route.boarding_options);
    if (options_status == TFX_LIMIT_REACHED) REJECT(IT_TERMINAL_LIMIT);
    if (options_status != TFX_OK) return options_status;
    board = result.route.boarding_options.stop;
    if (board.line != v.line || board.stop_index != v.stop_index || board.stop_count <= v.stop_index)
        return TFX_INVALID_STATE;
    result.route.replacement[result.route.replacement_count++] = board;
    /* Cross-platform boarding is limited to one physical station. Grouped
     * stations can include separate road/rail facilities; do not bridge them. */
    if (old_alight.has_alternative_terminals) REJECT(IT_ALTERNATIVE_TERMINALS);
    if (old_board.station_group != board.station_group || old_board.station_entity != board.station_entity ||
        old_board.station_index != board.station_index) REJECT(IT_BOARDING_MISMATCH);
    if (!v.arrival_locked) REJECT(IT_ACTUAL_TERMINAL_MISMATCH);
    const tfx_stop_terminals *options = &result.route.boarding_options;
    if (!options->count || options->count > TFX_STOP_TERMINALS_SIZE ||
        options->terminals[0].station_index != board.station_index ||
        options->terminals[0].station_entity != board.station_entity ||
        options->terminals[0].terminal != board.terminal ||
        (!!board.has_alternative_terminals != (options->count > 1))) return TFX_INVALID_STATE;
    int selected = 0;
    for (uint32_t i = 0; i < options->count; ++i) {
        const tfx_stop_terminal *t = &options->terminals[i];
        if (t->station_index < 0 || t->station_entity < 0 || t->terminal < 0) return TFX_INVALID_STATE;
        if (t->station_index == v.arrival_station_index && t->terminal == v.arrival_terminal) {
            result.route.actual_boarding = *t; selected = 1;
        }
    }
    if (!selected || result.route.actual_boarding.station_entity != old_board.station_entity)
        REJECT(IT_ACTUAL_TERMINAL_MISMATCH);
    result.cross_platform = result.route.actual_boarding.terminal != old_board.terminal;
    result.alternative_platform = v.arrival_station_index != board.station_index || v.arrival_terminal != board.terminal;
    /* A substitute follows an ordered subsequence of the original leg. Express
     * services may skip intermediate stops; extra stops/detours are excluded.
     * The first visit to the original alighting group must use the same station. */
    int32_t matched = 0;
    for (int32_t offset = 1; offset <= hops; ++offset) {
        if (offset >= board.stop_count) REJECT(IT_ALIGHTING_MISMATCH); /* No complete circuit. */
        int32_t index = (int32_t)(((int64_t)v.stop_index + offset) % board.stop_count);
        READ(api->v4.v3.v2.read_line_stop(context, v.line, index, &alight));
        if (alight.line != v.line || alight.stop_index != index ||
            alight.stop_count != board.stop_count || alight.station_group < 0) return TFX_INVALID_STATE;
        result.route.replacement[result.route.replacement_count++] = alight;
        int32_t next = matched + 1;
        while (next <= hops && original[next].station_group != alight.station_group) ++next;
        if (next > hops) REJECT(hops == 1 ? IT_ALIGHTING_MISMATCH : IT_ROUTE_DIVERGENCE);
        matched = next;
        if (matched == hops) {
            if (alight.has_alternative_terminals) REJECT(IT_ALTERNATIVE_TERMINALS);
            if (!same_station(&old_alight, &alight)) REJECT(IT_ALIGHTING_MISMATCH);
            result.alighting_stop = index;
            break;
        }
    }
    if (result.alighting_stop < 0) REJECT(IT_ALIGHTING_MISMATCH);
    result.boarding_station = result.route.actual_boarding.station_entity;
    result.boarding_terminal = result.route.actual_boarding.terminal;
    result.alighting_station = alight.station_entity; result.alighting_terminal = alight.terminal;
    result.cross_platform_alighting = old_alight.terminal != alight.terminal;
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
    tfx_terminal boarding = {.struct_size = sizeof(boarding)}, alighting = {.struct_size = sizeof(alighting)};
    READ(api->read_terminal(context, result.boarding_station, result.boarding_terminal, &boarding));
    if (boarding.station_entity != result.boarding_station || boarding.terminal != result.boarding_terminal ||
        boarding.terminal_count <= result.boarding_terminal) return TFX_INVALID_STATE;
    int member = 0;
    if (platforms.count > TFX_PERSON_PLATFORMS_SIZE) return TFX_INVALID_STATE;
    for (uint32_t i = 0; i < platforms.count; ++i)
        if (platforms.platforms[i].station_entity == old_board.station_entity &&
            platforms.platforms[i].terminal >= 0 && platforms.platforms[i].terminal < boarding.terminal_count)
            member = 1;
    if (!member) REJECT(IT_NO_PLATFORM_MEMBERSHIP);
    READ(api->read_terminal(context, alight.station_entity, alight.terminal, &alighting));
    if (alighting.station_entity != alight.station_entity || alighting.terminal != alight.terminal ||
        alighting.terminal_count <= alight.terminal) return TFX_INVALID_STATE;
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
