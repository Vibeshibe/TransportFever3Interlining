/* Exercise the real candidate consumer and policy through a copied SDK fixture. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "candidate_probe.h"
#include "probe_limits.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL %d: %s\n", __LINE__, #x); exit(1); } } while (0)
static tfx_client client;
static uint64_t sequence;
#define FIXTURE_PEOPLE (TFX_PASSENGER_LIMIT + 1)
static unsigned world, staged, boarded[TFX_PASSENGER_LIMIT], arrived[TFX_PASSENGER_LIMIT], queries, max_queries;
static unsigned no_capacity, wrong_vehicle, fail_result, pending_result, query_error, wrong_destination;
static int32_t source_line = 501, substitute_line = 902, leg_hops = 1, source_board = 1, substitute_board = 1;
static int32_t waiting_sentinel;
static unsigned route_changed, discovery_removed, screening_misses, busy_vehicles, active_stop_only, vehicle_missing, arriving_missing;
static unsigned missing_worlds[FIXTURE_PEOPLE], invalidated_worlds[FIXTURE_PEOPLE], life_error[FIXTURE_PEOPLE];
static unsigned cleared_home_worlds[FIXTURE_PEOPLE];
static unsigned lifecycle_calls[FIXTURE_PEOPLE], journey_calls[FIXTURE_PEOPLE];
static unsigned changed_home, reassigned, bad_identity, baseline_unassigned, home_target;
static tfx_waiting_leg_change requests[TFX_PASSENGER_LIMIT];
static char output[131072];
static void log_line(const char *s) { CHECK(strlen(output) + strlen(s) < sizeof(output)); strcat(output, s); }
#define BEGIN(out) do { CHECK(c == 7 && (out)->struct_size == sizeof(*(out))); ++queries; } while (0)
static tfx_status registration(const tfx_client *p) { client = *p; return TFX_OK; }
static tfx_status observation(tfx_context c, tfx_observation *out) {
    BEGIN(out); out->dispatch_sequence = sequence; return TFX_OK;
}
static tfx_status waiting(tfx_context c, int32_t id, tfx_waiting_leg *out) {
    BEGIN(out); CHECK(id >= 100 && id < 100 + FIXTURE_PEOPLE);
    /* Always return original waiting state, even for a changed person: discovery
     * must skip remembered passengers in a lagging replica without re-switching. */
    *out = (tfx_waiting_leg){.struct_size = sizeof(*out), .person = id, .destination = 777,
        .line = id == 100 + TFX_PASSENGER_LIMIT ? substitute_line : source_line, .boarding_stop = id == 100 + TFX_PASSENGER_LIMIT ? substitute_board : source_board,
        .alighting_stop = (id == 100 + TFX_PASSENGER_LIMIT ? substitute_board : source_board) + leg_hops, .vehicle = -1};
    if (waiting_sentinel && (id == 100 || id == 100 + TFX_PASSENGER_LIMIT)) out->vehicle = waiting_sentinel;
    if (screening_misses && id < 103) out->boarding_stop = 0;
    return TFX_OK;
}
static tfx_status scan(tfx_context c, int32_t first, tfx_waiting_page *out) {
    BEGIN(out); (void)first;
    *out = (tfx_waiting_page){.struct_size = sizeof(*out), .next_entity = -1};
    /* Keep one demand source for the substitute; paginate larger populations. */
    unsigned i = (unsigned)first;
    for (; i < TFX_PASSENGER_LIMIT && out->count < TFX_WAITING_PAGE_SIZE - 1; ++i)
        out->persons[out->count++] = 100 + (int32_t)i;
    out->persons[out->count++] = 100 + TFX_PASSENGER_LIMIT;
    if (i < TFX_PASSENGER_LIMIT) out->next_entity = (int32_t)i;
    return TFX_OK;
}
static tfx_status vehicle(tfx_context c, int32_t id, tfx_vehicle *out) {
    BEGIN(out); CHECK(id >= 96 && id <= 99);
    if (vehicle_missing) return TFX_NOT_FOUND;
    *out = (tfx_vehicle){.struct_size = sizeof(*out), .vehicle = id, .line = substitute_line,
        .stop_index = substitute_board, .state = id == 99 ? 2 : 1, .load_state = 3, .arrival_station_index = 0,
        .arrival_terminal = 1, .arrival_locked = 1, .passenger_capacity = TFX_PASSENGER_LIMIT + 5,
        .passengers_onboard = no_capacity ? TFX_PASSENGER_LIMIT + 5 : staged, .passenger_places_left = no_capacity ? 0 : TFX_PASSENGER_LIMIT + 5 - staged};
    return TFX_OK;
}
static tfx_status stop(tfx_context c, int32_t line, int32_t index, tfx_line_stop *out) {
    BEGIN(out);
    if (discovery_removed || (arriving_missing && index == substitute_board)) return TFX_NOT_FOUND;
    int32_t position = index - (line == substitute_line ? substitute_board : source_board) + 1;
    *out = (tfx_line_stop){.struct_size = sizeof(*out), .line = line, .stop_index = index,
        .stop_count = 6, .station_group = 1000 + position, .station_entity = 2000 + position,
        .station_index = 0, .terminal = 1, .passenger_load_enabled = 1,
        .passenger_max_load_bits = 0x3f800000};
    if (route_changed == 1 && line == substitute_line && index == 1 + leg_hops) out->terminal = 9;
    if (route_changed == 2 && index == 2) out->station_group += 50;
    return TFX_OK;
}
static tfx_status loading(tfx_context c, int32_t line, int32_t index, tfx_stop_vehicles *out) {
    BEGIN(out);
    *out = (tfx_stop_vehicles){.struct_size = sizeof(*out), .line = line,
        .stop_index = index, .count = line == substitute_line ? 1 : 0, .vehicles = {99}};
    if (active_stop_only && index != 1) out->count = 0;
    if (busy_vehicles && out->count) { out->count = 4; for (unsigned i = 0; i < 4; ++i) out->vehicles[i] = 96 + (int32_t)i; }
    return TFX_OK;
}
static tfx_status platforms(tfx_context c, int32_t id, tfx_person_platforms *out) {
    BEGIN(out);
    *out = (tfx_person_platforms){.struct_size = sizeof(*out), .person = id,
        .count = 1, .platforms = {{2001, 1}}}; return TFX_OK;
}
static tfx_status terminal(tfx_context c, int32_t station, int32_t index, tfx_terminal *out) {
    BEGIN(out);
    *out = (tfx_terminal){.struct_size = sizeof(*out), .station_entity = station,
        .terminal = index, .passengers_load = 1, .passengers_unload = 1}; return TFX_OK;
}
static tfx_status presence(tfx_context c, int32_t id, tfx_entity_presence *out) {
    BEGIN(out); CHECK(id == 777);
    *out = (tfx_entity_presence){.struct_size = sizeof(*out), .entity = id, .has_components = 1}; return TFX_OK;
}
static tfx_status request(tfx_context c, const tfx_waiting_leg_change *q, uint64_t *ticket) {
    CHECK(c == 7 && staged < TFX_PASSENGER_LIMIT);
    for (unsigned i = 0; i < staged; ++i) {
        CHECK(boarded[i] == 3 && requests[i].expected.person != q->expected.person);
    }
    /* Simulate staging reads + fresh full validation + backend pre/post queries.
     * Host timing and tokens themselves are exercised by host/query tests. */
    queries += 2;
    CHECK(q->revalidate(c, q) == TFX_OK);
    queries += 5;
    requests[staged++] = *q; *ticket = staged;
    return TFX_OK;
}
static tfx_status result(tfx_context c, uint64_t ticket, tfx_change_result *out) {
    BEGIN(out); CHECK(ticket && ticket <= staged);
    *out = (tfx_change_result){.struct_size = sizeof(*out), .ticket = ticket,
        .person = requests[ticket - 1].expected.person, .engine_attempted = 1,
        .status = pending_result ? TFX_PENDING : fail_result ? TFX_INVALID_STATE : TFX_OK}; return TFX_OK;
}
static tfx_status journey(tfx_context c, int32_t id, tfx_person_journey *out) {
    BEGIN(out); ++journey_calls[id - 100]; if (query_error) return TFX_NOT_FOUND;
    unsigned index = 0;
    while (index < staged && requests[index].expected.person != id) ++index;
    CHECK(index < staged);
    unsigned bit = 1u << (world - 1);
    *out = (tfx_person_journey){.struct_size = sizeof(*out), .world_id = world, .person = id,
        .destination = wrong_destination ? 999 : 777, .line = substitute_line, .boarding_stop = substitute_board,
        .alighting_stop = substitute_board + leg_hops, .vehicle = wrong_vehicle ? 88 : 99,
        .flags = arrived[index] & bit ? TFX_JOURNEY_AT_BUILDING :
            boarded[index] & bit ? TFX_JOURNEY_ONBOARD : TFX_JOURNEY_WAITING};
    return TFX_OK;
}
static tfx_status lifecycle(tfx_context c, int32_t id, tfx_person_lifecycle *out) {
    BEGIN(out);
    ++lifecycle_calls[id - 100];
    if (life_error[id - 100]) return TFX_INVALID_STATE;
    unsigned bit = 1u << (world - 1);
    *out = (tfx_person_lifecycle){.struct_size = sizeof(*out), .world_id = world,
        .person = bad_identity ? id + 1 : id, .flags = 3, .target = wrong_destination ? 999 : 777,
        .destinations = {changed_home ? 998 : 555, -1, baseline_unassigned ? -1 : 777}};
    if (missing_worlds[id - 100] & bit) out->flags = 1; /* ID reused by a non-person entity. */
    if (invalidated_worlds[id - 100] & bit) { out->target = 555; out->destinations[2] = -1; }
    if (reassigned) { out->target = 999; out->destinations[2] = 999; }
    if (home_target) { out->destinations[0] = 777; out->destinations[2] = -1; }
    if (cleared_home_worlds[id - 100] & bit) out->destinations[0] = -1;
    return TFX_OK;
}
tfx_status tfx_get_api(uint32_t version, tfx_api *base) {
    CHECK(version == 5 || version == 7);
    base->abi_version = version; base->capabilities = version == 5 ? 31 : 255;
    base->register_client = registration; base->read_observation = observation;
    tfx_api_v5 *api = (tfx_api_v5 *)base;
    api->v4.v3.v2.read_waiting_leg = waiting; api->v4.v3.v2.scan_waiting_people = scan;
    api->v4.v3.v2.read_line_stop = stop; api->v4.v3.read_vehicle = vehicle;
    api->v4.v3.read_stop_vehicles = loading; api->v4.read_person_platforms = platforms;
    api->read_terminal = terminal; api->read_entity_presence = presence;
    if (version == 7) {
        ((tfx_api_v7 *)base)->read_person_lifecycle = lifecycle;
        tfx_api_v6 *changes = (tfx_api_v6 *)base;
        changes->request_waiting_leg_change = request; changes->read_change_result = result;
        changes->read_person_journey = journey;
    }
    return TFX_OK;
}
static void reset(void) {
    sequence = staged = no_capacity = wrong_vehicle = fail_result = pending_result = query_error = wrong_destination = 0;
    changed_home = reassigned = bad_identity = baseline_unassigned = home_target = 0;
    source_line = 501; substitute_line = 902; leg_hops = 1; source_board = substitute_board = 1;
    waiting_sentinel = 0;
    route_changed = discovery_removed = screening_misses = busy_vehicles = active_stop_only = vehicle_missing = arriving_missing = 0;
    memset(missing_worlds, 0, sizeof(missing_worlds)); memset(invalidated_worlds, 0, sizeof(invalidated_worlds));
    memset(life_error, 0, sizeof(life_error)); memset(lifecycle_calls, 0, sizeof(lifecycle_calls));
    memset(cleared_home_worlds, 0, sizeof(cleared_home_worlds));
    memset(journey_calls, 0, sizeof(journey_calls));
    memset(boarded, 0, sizeof(boarded)); memset(arrived, 0, sizeof(arrived)); output[0] = 0;
    CHECK(tfx_register_candidate_probe(log_line) == TFX_OK);
}
static void tick(void) {
    ++sequence; world = (unsigned)((sequence - 1) % 2) + 1; queries = 0;
    client.before_terminal_selection(7, NULL);
    if (queries > max_queries) max_queries = queries;
    CHECK(queries <= 64);
}
static void ticks(unsigned count) { for (unsigned i = 0; i < count; ++i) tick(); }
/* Keep spare allowance while establishing both-world boarding. */
static void first_boarded(void) {
    reset(); tick(); CHECK(staged == 1); boarded[0] = 3; no_capacity = 1;
    ticks(10); CHECK(staged == 1 && strstr(output, "pilot_boarded_both person=100"));
    CHECK(sequence % 4 == 3); /* Next callback tracks world2, not discovery. */
}
static void stage_until(unsigned count) {
    for (unsigned n = 0; n < 800 && staged < count; ++n) {
        for (unsigned i = 0; i < staged; ++i) boarded[i] = 3;
        tick();
    }
    if (staged != count) fprintf(stderr, "Staging target=%u actual=%u sequence=%llu\n%s", count, staged, (unsigned long long)sequence, output + (strlen(output) > 4000 ? strlen(output) - 4000 : 0));
    CHECK(staged == count);
}
#ifndef TFX_TEST_SAMPLED
static void home_clear_tests(void) {
    first_boarded(); cleared_home_worlds[0] = 2; tick();
    CHECK(!tfx_pilot_complete() && strstr(output, "pilot_home_pending person=100") &&
        !strstr(output, "pilot_lifecycle_resolved"));
    no_capacity = 0; ticks(8); CHECK(staged == 1);
    missing_worlds[0] = 2; ticks(2); CHECK(staged == 1);
    missing_worlds[0] = 3; ticks(2);
    CHECK(!tfx_pilot_complete() && strstr(output, "pilot_lifecycle_resolved person=100") &&
        strstr(output, "pilot_tracking_resumed"));
    unsigned reads = lifecycle_calls[0], journeys = journey_calls[0];
    cleared_home_worlds[0] = missing_worlds[0] = 0; life_error[0] = 1;
    ticks(10); CHECK(staged == 2 && requests[1].expected.person != 100);
    CHECK(lifecycle_calls[0] == reads && journey_calls[0] == journeys);

    /* Matching home loss closes as its own outcome while the person remains
     * onboard. Never claim disappearance/arrival, re-read the ID, or refund it. */
    first_boarded(); cleared_home_worlds[0] = 2; tick(); no_capacity = 0;
    ticks(4); CHECK(staged == 1 && !strstr(output, "pilot_lifecycle_resolved"));
    cleared_home_worlds[0] = 3; ticks(2);
    CHECK(strstr(output, "outcome=home_invalidated world_mask=3 confirmed=1 arrival=0") &&
        strstr(output, "pilot_tracking_resumed"));
    reads = lifecycle_calls[0]; journeys = journey_calls[0];
    cleared_home_worlds[0] = 0; life_error[0] = 1;
    ticks(10); CHECK(staged == 2 && requests[1].expected.person != 100);
    CHECK(lifecycle_calls[0] == reads && journey_calls[0] == journeys);
    stage_until(5); boarded[4] = 3; ticks(4);
    for (unsigned i = 1; i < 5; ++i) arrived[i] = 3;
    ticks(2); CHECK(tfx_pilot_complete() && strstr(output, "reason=all_outcomes_observed staged=5") &&
        strstr(output, "destination_world_mask=0 removed_world_mask=0 invalidated_world_mask=0 outcome=home_invalidated home_cleared_world_mask=3"));
    ticks(10); CHECK(staged == 5);

    /* Removal in one world is not matching home invalidation in both. */
    first_boarded(); cleared_home_worlds[0] = 2; tick(); missing_worlds[0] = 2; ticks(2);
    cleared_home_worlds[0] = 3; tick();
    CHECK(!strstr(output, "pilot_lifecycle_resolved") && !tfx_pilot_complete());
    missing_worlds[0] = 3; ticks(2);
    CHECK(strstr(output, "outcome=passenger_removed arrival=0 attempt_retained=1") &&
        !strstr(output, "outcome=home_invalidated"));
    first_boarded(); cleared_home_worlds[0] = 2; tick(); missing_worlds[0] = 1;
    ticks(TFX_HOME_CLEAR_GRACE_CALLBACKS);
    CHECK(tfx_pilot_complete() && strstr(output, "home_clear_timeout") &&
        !strstr(output, "pilot_lifecycle_resolved"));
    first_boarded(); cleared_home_worlds[0] = 2; tick(); missing_worlds[0] = 2; ticks(2);
    missing_worlds[0] = 0; ticks(2);
    CHECK(tfx_pilot_complete() && strstr(output, "passenger_identity_unresolved"));

    /* One-world clearing cannot release the pilot; repeated observations must
     * not refresh its original confirmation deadline. */
    first_boarded(); cleared_home_worlds[0] = 2; tick(); no_capacity = 0;
    ticks(TFX_HOME_CLEAR_GRACE_CALLBACKS - 1);
    CHECK(!tfx_pilot_complete() && staged == 1);
    tick(); CHECK(tfx_pilot_complete() && staged == 1 && strstr(output, "reason=home_clear_timeout") &&
        strstr(output, "outcome=unresolved home_cleared_world_mask=2") &&
        !strstr(output, "pilot_lifecycle_resolved"));
    first_boarded(); cleared_home_worlds[0] = 2; tick();
    ticks(TFX_HOME_CLEAR_GRACE_CALLBACKS - 1); cleared_home_worlds[0] = 3; tick();
    CHECK(tfx_pilot_complete() && strstr(output, "home_clear_timeout") &&
        !strstr(output, "pilot_lifecycle_resolved"));

    /* Neither restoration nor a different home can rebase a tracked identity. */
    first_boarded(); cleared_home_worlds[0] = 2; tick(); cleared_home_worlds[0] = 0; ticks(2);
    CHECK(tfx_pilot_complete() && strstr(output, "passenger_identity_unresolved"));
    first_boarded(); cleared_home_worlds[0] = 2; tick(); changed_home = 1; tick();
    CHECK(tfx_pilot_complete() && strstr(output, "passenger_identity_unresolved"));
    reset(); tick(); cleared_home_worlds[0] = 3; tick();
    CHECK(tfx_pilot_complete() && !strstr(output, "pilot_home_pending"));
    first_boarded(); cleared_home_worlds[0] = 3; wrong_destination = 1; tick();
    CHECK(tfx_pilot_complete() && !strstr(output, "pilot_home_pending"));
    first_boarded(); cleared_home_worlds[0] = 3; invalidated_worlds[0] = 3; tick();
    CHECK(tfx_pilot_complete() && !strstr(output, "pilot_home_pending"));
    reset(); home_target = 1; tick(); boarded[0] = 3; no_capacity = 1; ticks(10);
    cleared_home_worlds[0] = 3; tick();
    CHECK(tfx_pilot_complete() && !strstr(output, "pilot_home_pending"));

    /* The other replica must not mix an arrival/invalidation into pending loss. */
    first_boarded(); cleared_home_worlds[0] = 2; tick(); arrived[0] = 1; tick();
    CHECK(tfx_pilot_complete() && strstr(output, "arrival_lifecycle_disagreement"));
    first_boarded(); cleared_home_worlds[0] = 2; tick(); invalidated_worlds[0] = 1; tick();
    CHECK(tfx_pilot_complete() && strstr(output, "home_clear_lifecycle_disagreement"));
    first_boarded(); arrived[0] = 2; tick(); cleared_home_worlds[0] = 1; tick();
    CHECK(tfx_pilot_complete() && !strstr(output, "pilot_home_pending"));

    /* Every pending passenger must resolve, with no release of permanent errors. */
    reset(); stage_until(2); boarded[1] = 3; no_capacity = 1; ticks(8);
    cleared_home_worlds[0] = cleared_home_worlds[1] = 2; ticks(4);
    missing_worlds[0] = 3; ticks(2); no_capacity = 0; ticks(4); CHECK(staged == 2);
    cleared_home_worlds[1] = 3; ticks(10); CHECK(staged == 3 && strstr(output, "pilot_tracking_resumed"));
    reset(); stage_until(2); boarded[1] = 3; no_capacity = 1; ticks(8);
    cleared_home_worlds[0] = 2; ticks(4); life_error[1] = 1; tick();
    cleared_home_worlds[0] = 3; ticks(2); life_error[1] = 0; no_capacity = 0; ticks(10);
    CHECK(staged == 2 && !strstr(output, "pilot_tracking_resumed"));

    first_boarded(); sequence = TFX_OBSERVATION_LIMIT - 3;
    cleared_home_worlds[0] = 2; tick(); missing_worlds[0] = 3; ticks(2);
    CHECK(tfx_pilot_complete() && staged == 1 && strstr(output, "reason=callback_limit"));
    puts("PASS: both-world home invalidation resumes other IDs without removal/arrival claims or cap refunds; unconfirmed loss times out at 32 callbacks and conflicting evidence still stops.");
}
#if TFX_RESERVE_LAST_FOR_LIFECYCLE
static void reserve_tests(void) {
    reset(); stage_until(4);
    for (unsigned i = 0; i < 4; ++i) boarded[i] = 3;
    ticks(20); CHECK(staged == 4 && !tfx_pilot_complete());
    for (unsigned i = 1; i < 4; ++i) arrived[i] = 3;
    ticks(8); CHECK(staged == 4); /* Arrivals alone do not release the reserved attempt. */
    cleared_home_worlds[0] = 2; ticks(8);
    CHECK(staged == 4 && !tfx_pilot_complete() && strstr(output, "pilot_home_pending"));
    missing_worlds[0] = 2; ticks(8); CHECK(staged == 4 && !tfx_pilot_complete());
    missing_worlds[0] = 3; ticks(10);
    CHECK(staged == 5 && !tfx_pilot_complete() && strstr(output, "pilot_tracking_resumed"));
    const char *resolved = strstr(output, "pilot_lifecycle_resolved person=100"), *fifth = strstr(output, "ordinal=5");
    CHECK(resolved && fifth && resolved < fifth);
    boarded[4] = 3; ticks(4); arrived[4] = 3; ticks(2);
    CHECK(tfx_pilot_complete() && strstr(output, "reason=all_outcomes_observed staged=5"));
    CHECK(strstr(output, "destination_world_mask=0 removed_world_mask=3 invalidated_world_mask=0 outcome=passenger_removed"));
    ticks(20); CHECK(staged == 5); /* Never refund the retired passenger's attempt. */
    /* The onboard passenger may retain its cleared home indefinitely; matching
     * home invalidation releases the reserved fifth without claiming removal. */
    reset(); stage_until(4);
    for (unsigned i = 0; i < 4; ++i) boarded[i] = 3;
    ticks(8); cleared_home_worlds[0] = 2; ticks(8); CHECK(staged == 4);
    cleared_home_worlds[0] = 3; ticks(10); CHECK(staged == 5);
    resolved = strstr(output, "outcome=home_invalidated arrival=0 attempt_retained=1");
    fifth = strstr(output, "ordinal=5"); CHECK(resolved && fifth && resolved < fifth);
    boarded[4] = 3; ticks(4);
    for (unsigned i = 1; i < 5; ++i) arrived[i] = 3;
    ticks(2); CHECK(tfx_pilot_complete() && strstr(output, "reason=all_outcomes_observed staged=5") &&
        strstr(output, "destination_world_mask=0 removed_world_mask=0 invalidated_world_mask=0 outcome=home_invalidated home_cleared_world_mask=3"));
    ticks(20); CHECK(staged == 5);
    reset(); stage_until(4);
    for (unsigned i = 0; i < 4; ++i) boarded[i] = 3;
    ticks(8); for (unsigned i = 0; i < 4; ++i) arrived[i] = 3;
    ticks(8); CHECK(staged == 4 && !tfx_pilot_complete());
    sequence = TFX_OBSERVATION_LIMIT - 1; tick();
    CHECK(tfx_pilot_complete() && staged == 4 && strstr(output, "reason=callback_limit"));
    first_boarded(); /* Both-world confirmation on the cap cannot stage another passenger. */
    missing_worlds[0] = 2; tick(); missing_worlds[0] = 3;
    sequence = TFX_OBSERVATION_LIMIT - 2; tick(); tick();
    CHECK(tfx_pilot_complete() && staged == 1 && strstr(output, "reason=callback_limit"));
    puts("PASS: trial reserve waits for confirmed lifecycle with unused allowance, then permits the fifth distinct attempt; no cap refund or final-callback proposal.");
}
#endif
int main(void) {
#if TFX_RESERVE_LAST_FOR_LIFECYCLE
    reserve_tests();
#else
    reset(); tick(); CHECK(staged == 1);
    ticks(20); CHECK(staged == 1);
    boarded[0] = 1; ticks(10); CHECK(staged == 1);
    for (unsigned i = 0; i < 5; ++i) {
        CHECK(staged == i + 1); boarded[i] = 3; ticks(10);
        CHECK(staged == (i < 4 ? i + 2 : 5));
    }
    ticks(10); CHECK(staged == 5 && !tfx_pilot_complete());
    for (unsigned i = 0; i < 5; ++i) arrived[i] = 3;
    ticks(2); CHECK(tfx_pilot_complete() && strstr(output, "reason=all_destinations_observed staged=5"));
    first_boarded(); CHECK(strstr(output, "reason=no_capacity"));
    no_capacity = 0; ticks(10); CHECK(staged == 2);
    reset(); tick(); boarded[0] = 3; wrong_vehicle = 1; tick();
    CHECK(tfx_pilot_complete() && staged == 1 && strstr(output, "unexpected_first_boarding"));
    reset(); tick(); fail_result = 1; tick();
    CHECK(tfx_pilot_complete() && strstr(output, "change_rejected_or_failed"));
    reset(); tick(); wrong_destination = 1; tick();
    CHECK(tfx_pilot_complete() && strstr(output, "destination_changed_before_arrival"));
    reset(); tick(); query_error = 1; ticks(8);
    CHECK(tfx_pilot_complete() && staged == 1 && strstr(output, "reason=query_errors") &&
        strstr(output, "pilot_journey_error person=100 destination=777"));
    reset(); tick(); sequence = TFX_OBSERVATION_LIMIT - 1; tick();
    CHECK(tfx_pilot_complete() && strstr(output, "reason=callback_limit") &&
        strstr(output, "onboard_world_mask=0 destination_world_mask=0"));
    reset(); sequence = TFX_OBSERVATION_LIMIT - 1; tick(); CHECK(tfx_pilot_complete() && staged == 0);
    reset(); baseline_unassigned = 1; tick();
    CHECK(tfx_pilot_complete() && staged == 0 && strstr(output, "lifecycle_baseline_unassigned"));

    /* Resolve each kind after boarding and then actually stage a different ID. */
    for (unsigned invalidation = 0; invalidation < 2; ++invalidation) {
        first_boarded();
        unsigned *event = invalidation ? &invalidated_worlds[0] : &missing_worlds[0];
        *event = 2; ticks(8); CHECK(staged == 1 && !tfx_pilot_complete());
        CHECK(!strstr(output, "pilot_tracking_resumed"));
        *event = 3; ticks(2);
        CHECK(!tfx_pilot_complete() && strstr(output, "pilot_lifecycle_resolved person=100") &&
            strstr(output, "pilot_tracking_resumed"));
        unsigned reads = lifecycle_calls[0], journeys = journey_calls[0];
        no_capacity = 0; ticks(10); CHECK(staged == 2);
        /* Closed IDs remain excluded even if reused or assigned a later journey. */
        *event = 0; life_error[0] = 1; ticks(4);
        CHECK(lifecycle_calls[0] == reads && journey_calls[0] == journeys && !tfx_pilot_complete());
        stage_until(5); boarded[4] = 3; ticks(4);
        for (unsigned i = 1; i < 5; ++i) arrived[i] = 3;
        ticks(2); CHECK(tfx_pilot_complete() && strstr(output, "reason=all_outcomes_observed staged=5"));
        CHECK(strstr(output, invalidation ?
            "destination_world_mask=0 removed_world_mask=0 invalidated_world_mask=3 outcome=destination_invalidated" :
            "destination_world_mask=0 removed_world_mask=3 invalidated_world_mask=0 outcome=passenger_removed"));
        ticks(20); CHECK(staged == 5);
    }
    /* One confirmation cannot release another passenger's pending lifecycle. */
    reset(); stage_until(2); boarded[1] = 3; no_capacity = 1; ticks(8);
    missing_worlds[0] = invalidated_worlds[1] = 2; ticks(8);
    missing_worlds[0] = 3; ticks(2); CHECK(!strstr(output, "pilot_tracking_resumed"));
    no_capacity = 0; ticks(8); CHECK(staged == 2);
    invalidated_worlds[1] = 3; ticks(10); CHECK(staged == 3 && strstr(output, "pilot_tracking_resumed"));
    /* Nor can confirmation release the permanent block from an unrelated error. */
    reset(); stage_until(2); boarded[1] = 3; no_capacity = 1; ticks(8);
    missing_worlds[0] = 2; ticks(2); life_error[1] = 1; tick();
    missing_worlds[0] = 3; ticks(2); life_error[1] = 0; no_capacity = 0; ticks(20);
    CHECK(staged == 2 && !tfx_pilot_complete() && !strstr(output, "pilot_tracking_resumed"));

    reset(); tick(); missing_worlds[0] = 3; ticks(2);
    CHECK(tfx_pilot_complete() && staged == 1 && strstr(output, "lifecycle_before_confirmed_boarding") &&
        strstr(output, "removed_world_mask=3 invalidated_world_mask=0 outcome=unresolved"));
    first_boarded(); missing_worlds[0] = 2; tick(); invalidated_worlds[0] = 1; tick();
    CHECK(tfx_pilot_complete() && strstr(output, "lifecycle_world_disagreement"));
    first_boarded(); missing_worlds[0] = 2; tick(); missing_worlds[0] = 0; ticks(2);
    CHECK(tfx_pilot_complete() && strstr(output, "passenger_identity_unresolved"));
    first_boarded(); invalidated_worlds[0] = 2; tick(); invalidated_worlds[0] = 0; ticks(2);
    CHECK(tfx_pilot_complete() && strstr(output, "destination_assignment_unresolved"));
    first_boarded(); arrived[0] = 2; tick(); missing_worlds[0] = 1; tick();
    CHECK(tfx_pilot_complete() && strstr(output, "arrival_lifecycle_disagreement"));
    first_boarded(); missing_worlds[0] = 2; tick(); arrived[0] = 1; tick();
    CHECK(tfx_pilot_complete() && strstr(output, "arrival_lifecycle_disagreement"));
    reset(); tick(); changed_home = 1; tick();
    CHECK(tfx_pilot_complete() && strstr(output, "passenger_identity_unresolved") &&
        strstr(output, "pilot_identity_error person=100 sequence=") &&
        strstr(output, "expected_home=555 observed_home=998 destinations=998,-1,777 target=777 removed_world_mask=0 outcome=unresolved"));
    reset(); tick(); reassigned = 1; tick();
    CHECK(tfx_pilot_complete() && strstr(output, "destination_assignment_unresolved"));
    reset(); tick(); bad_identity = 1; tick();
    CHECK(tfx_pilot_complete() && strstr(output, "unexpected_lifecycle_identity"));
    reset(); tick(); life_error[0] = 1; ticks(8);
    CHECK(tfx_pilot_complete() && staged == 1 && strstr(output, "reason=query_errors") &&
        strstr(output, "pilot_lifecycle_error person=100") && !strstr(output, "outcome=passenger_removed"));
    reset(); tick(); boarded[0] = 3; life_error[0] = 1; tick(); life_error[0] = 0; ticks(20);
    CHECK(staged == 1 && !tfx_pilot_complete());
    first_boarded(); arrived[0] = 3; ticks(8); missing_worlds[0] = 3; ticks(8);
    CHECK(!tfx_pilot_complete() && !strstr(output, "outcome=passenger_removed"));
    first_boarded(); missing_worlds[0] = 2; tick();
    sequence = TFX_OBSERVATION_LIMIT - 1; tick();
    CHECK(tfx_pilot_complete() && staged == 1 && strstr(output, "reason=callback_limit"));
    puts("PASS: lifecycle continuation after both-world boarding/confirmation; permanent error block, all pending passengers, ID retention, mixed outcomes, no cap refund, disagreement and prior-arrival guards.");
#endif
    home_clear_tests();
    reset(); leg_hops = 4; tick(); CHECK(staged == 1);
    CHECK(requests[0].expected.line == 501 && requests[0].line == 902 &&
        requests[0].expected.alighting_stop == 5 && requests[0].alighting_stop == 5);
    route_changed = 1; queries = 0;
    CHECK(requests[0].revalidate(7, &requests[0]) == TFX_POLICY_REJECTED);
    CHECK(queries <= 64);
    route_changed = 2; queries = 0; /* Both routes still match, but changed since staging. */
    CHECK(requests[0].revalidate(7, &requests[0]) == TFX_EXPECTED_STATE_CHANGED);
    CHECK(queries <= 64); route_changed = 0;
    reset(); leg_hops = 4; screening_misses = busy_vehicles = 1; tick();
    CHECK(staged == 1 && requests[0].expected.person == 103);
    reset(); discovery_removed = 1; ticks(20);
    CHECK(staged == 0 && !strstr(output, "candidate_query_error"));
    discovery_removed = 0; stage_until(1);
    CHECK(staged == 1); /* Removal/reappearance of demand lines is refreshed. */
    reset(); source_board = 0; substitute_board = 2; leg_hops = 2;
    tick(); CHECK(staged == 1 && requests[0].expected.boarding_stop == 0 &&
        requests[0].expected.alighting_stop == 2 && requests[0].boarding_stop == 2 && requests[0].alighting_stop == 4);
    boarded[0] = 3; ticks(2); CHECK(strstr(output, "pilot_boarded_both"));
    reset(); active_stop_only = 1; stage_until(1); CHECK(sequence > 1);
    CHECK(requests[0].line == substitute_line); /* Advances per-line stops despite alternating phases. */
    for (int32_t sentinel = -2; sentinel >= -3; --sentinel) {
        reset(); waiting_sentinel = sentinel; tick();
        CHECK(staged == 1 && requests[0].expected.person == 101 &&
            strstr(output, "pilot_waiting_excluded person=105") &&
            strstr(output, "pilot_waiting_excluded person=100") &&
            !strstr(output, "candidate_query_error"));
    }
    reset(); arriving_missing = 1; ticks(20);
    CHECK(staged == 0 && !strstr(output, "candidate_query_error"));
    arriving_missing = 0; stage_until(1);
    reset(); vehicle_missing = 1; ticks(20);
    CHECK(staged == 0 && !strstr(output, "candidate_query_error"));
    vehicle_missing = 0; stage_until(1);
    puts("PASS: arbitrary demand-discovered line IDs, multi-stop staging/revalidation, stale-line exclusion/recovery and worst screening budget.");
    printf("PASS: consumer and real policy max fixture query allowance=%u/64.\n", max_queries);
    return 0;
}

#else
static unsigned occurrences(const char *needle) {
    unsigned count = 0;
    for (const char *p = output; (p = strstr(p, needle)) != NULL; p += strlen(needle)) ++count;
    return count;
}
int main(void) {
    CHECK(TFX_PASSENGER_LIMIT == 32 && TFX_JOURNEY_SAMPLE_LIMIT == 4);
    reset(); stage_until(32); boarded[31] = 3; ticks(4);
    CHECK(!tfx_pilot_complete() && staged == 32);
    CHECK(occurrences("selected=1 policy=first_free_slot") == 4);
    CHECK(occurrences("selected=0 outcome=boarding_confirmed arrival=unobserved") == 28);
    unsigned reads[FIXTURE_PEOPLE], journeys[FIXTURE_PEOPLE];
    memcpy(reads, lifecycle_calls, sizeof(reads)); memcpy(journeys, journey_calls, sizeof(journeys));
    for (unsigned i = 4; i < 32; ++i) life_error[requests[i].expected.person - 100] = 1;
    ticks(20);
    for (unsigned i = 4; i < 32; ++i) {
        unsigned id = (unsigned)requests[i].expected.person - 100;
        CHECK(lifecycle_calls[id] == reads[id] && journey_calls[id] == journeys[id]);
    }
    for (unsigned i = 0; i < 4; ++i) arrived[i] = 3;
    ticks(2);
    CHECK(tfx_pilot_complete() && strstr(output, "reason=sampled_outcomes_observed staged=32"));
    CHECK(strstr(output, "staged=32 applied=32 boarded=32 sampled=4 sampled_arrivals=4 sampled_lifecycle=0 unsampled_boardings=28 active_trackers=0 active_samples=0 peak_trackers=5 pending_boarding=0"));
    CHECK(!strstr(output, "candidate_query_error"));
    ticks(20); CHECK(staged == 32);

    /* A completed sample frees a physical tracker, never the old person's ID. */
    reset(); stage_until(4); boarded[3] = 3; no_capacity = 1; ticks(4);
    unsigned first_id = (unsigned)requests[0].expected.person - 100;
    arrived[0] = 3; ticks(4);
    unsigned old_reads = lifecycle_calls[first_id], old_journeys = journey_calls[first_id];
    life_error[first_id] = 1; no_capacity = 0; stage_until(5);
    boarded[4] = 3; no_capacity = 1; ticks(4);
    CHECK(occurrences("selected=1 policy=first_free_slot") == 5);
    CHECK(lifecycle_calls[first_id] == old_reads && journey_calls[first_id] == old_journeys);
    no_capacity = 0; stage_until(6); boarded[5] = 3; no_capacity = 1; ticks(4);
    CHECK(occurrences("selected=0 outcome=boarding_confirmed") == 1);
    unsigned released = (unsigned)requests[5].expected.person - 100;
    old_reads = lifecycle_calls[released]; old_journeys = journey_calls[released];
    life_error[released] = 1;
    /* Release a sampled removal, then fill its slot while retired IDs stay excluded. */
    unsigned removed = (unsigned)requests[1].expected.person - 100;
    missing_worlds[removed] = 2; ticks(4); no_capacity = 0; ticks(4); CHECK(staged == 6);
    missing_worlds[removed] = 3; stage_until(7); boarded[6] = 3; no_capacity = 1; ticks(4);
    CHECK(occurrences("selected=1 policy=first_free_slot") == 6);
    CHECK(lifecycle_calls[released] == old_reads && journey_calls[released] == old_journeys);
    sequence = TFX_OBSERVATION_LIMIT - 1; tick();
    CHECK(strstr(output, "sampled=6 sampled_arrivals=1 sampled_lifecycle=1 unsampled_boardings=1"));
    CHECK(strstr(output, "reason=callback_limit staged=7") && !strstr(output, "candidate_query_error"));

    /* Pending slot 0 and finishing sample slot 1 must see the same free capacity
     * as the opposite physical ordering. */
    reset(); stage_until(4); boarded[3] = 3; no_capacity = 1; ticks(4);
    arrived[0] = 3; ticks(4); no_capacity = 0; stage_until(5);
    boarded[4] = 3; arrived[1] = 3; no_capacity = 1; ticks(4);
    CHECK(occurrences("selected=1 policy=first_free_slot") == 5 &&
        !strstr(output, "selected=0 outcome=boarding_confirmed"));

    /* Confirmation cannot expire one replica silently or accept a late success. */
    reset(); tick(); boarded[0] = 1; ticks(4);
    sequence = TFX_BOARDING_GRACE_CALLBACKS - 1; tick(); CHECK(!tfx_pilot_complete());
    boarded[0] = 3; tick();
    CHECK(tfx_pilot_complete() && staged == 1 && strstr(output, "reason=boarding_confirmation_timeout") &&
        !strstr(output, "pilot_boarded_both"));
    reset(); tick(); boarded[0] = 1; ticks(4);
    sequence = TFX_BOARDING_GRACE_CALLBACKS - 1; boarded[0] = 3; tick();
    CHECK(!tfx_pilot_complete() && strstr(output, "pilot_boarded_both"));
    reset(); tick(); pending_result = 1; ticks(4);
    sequence = TFX_BOARDING_GRACE_CALLBACKS; tick();
    CHECK(tfx_pilot_complete() && staged == 1 && strstr(output, "boarding_confirmation_timeout") &&
        strstr(output, "staged=1 applied=0 boarded=0 sampled=0"));
    reset(); tick(); fail_result = 1; tick();
    CHECK(tfx_pilot_complete() && staged == 1 && strstr(output, "change_rejected_or_failed"));
    CHECK(strstr(output, "staged=1 applied=0 boarded=0 sampled=0"));
    first_boarded(); cleared_home_worlds[0] = 2; tick(); no_capacity = 0;
    ticks(TFX_HOME_CLEAR_GRACE_CALLBACKS);
    CHECK(tfx_pilot_complete() && staged == 1 && strstr(output, "home_clear_timeout"));

    /* Existing real-policy worst-case remains within budget in sampled mode. */
    reset(); leg_hops = 4; screening_misses = busy_vehicles = 1; tick();
    CHECK(staged == 1 && requests[0].expected.person == 103);
    for (int32_t sentinel = -2; sentinel >= -3; --sentinel) {
        reset(); waiting_sentinel = sentinel; tick();
        CHECK(staged == 1 && requests[0].expected.person == 101 &&
            strstr(output, "pilot_waiting_excluded") && !strstr(output, "candidate_query_error"));
    }
    reset(); no_capacity = 1; ticks(600);
    CHECK(strstr(output, "pilot_discovery sequence=600 callbacks=600 scans=600") &&
        strstr(output, "pilot_totals sequence=600 staged=0 applied=0 boarded=0 sampled=0"));
    printf("PASS: 32 unique attempts, four sampled journeys plus one pending, sample reuse, no reads or duplicate changes after release, exact deadlines, final counts and metrics; peak=%u/64 queries.\n", max_queries);
    return 0;
}
#endif
