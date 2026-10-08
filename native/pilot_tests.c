/* Exercise the real candidate consumer and policy through a copied SDK fixture. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "candidate_probe.h"
#include "probe_limits.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL %d: %s\n", __LINE__, #x); exit(1); } } while (0)
static tfx_client client;
static uint64_t sequence;
static unsigned world, staged, boarded[5], arrived[5], queries, max_queries;
static unsigned no_capacity, wrong_vehicle, fail_result, query_error, wrong_destination;
static unsigned missing_worlds[6], invalidated_worlds[6], life_error[6];
static unsigned lifecycle_calls[6], journey_calls[6];
static unsigned changed_home, reassigned, bad_identity, baseline_unassigned;
static tfx_waiting_leg_change requests[5];
static char output[32768];
static void log_line(const char *s) { CHECK(strlen(output) + strlen(s) < sizeof(output)); strcat(output, s); }
#define BEGIN(out) do { CHECK(c == 7 && (out)->struct_size == sizeof(*(out))); ++queries; } while (0)
static tfx_status registration(const tfx_client *p) { client = *p; return TFX_OK; }
static tfx_status observation(tfx_context c, tfx_observation *out) {
    BEGIN(out); out->dispatch_sequence = sequence; return TFX_OK;
}
static tfx_status waiting(tfx_context c, int32_t id, tfx_waiting_leg *out) {
    BEGIN(out); CHECK(id >= 100 && id < 106);
    /* Always return original waiting state, even for a changed person: discovery
     * must skip remembered passengers in a lagging replica without re-switching. */
    *out = (tfx_waiting_leg){.struct_size = sizeof(*out), .person = id, .destination = 777,
        .line = 98278, .boarding_stop = 1, .alighting_stop = 2, .vehicle = -1};
    return TFX_OK;
}
static tfx_status scan(tfx_context c, int32_t first, tfx_waiting_page *out) {
    BEGIN(out); (void)first;
    *out = (tfx_waiting_page){.struct_size = sizeof(*out), .count = 6, .next_entity = -1,
        .persons = {100, 101, 102, 103, 104, 105}};
    return TFX_OK;
}
static tfx_status vehicle(tfx_context c, int32_t id, tfx_vehicle *out) {
    BEGIN(out); CHECK(id == 99);
    *out = (tfx_vehicle){.struct_size = sizeof(*out), .vehicle = id, .line = 97743,
        .stop_index = 1, .state = 2, .load_state = 3, .arrival_station_index = 0,
        .arrival_terminal = 1, .arrival_locked = 1, .passenger_capacity = 10,
        .passengers_onboard = no_capacity ? 10 : staged, .passenger_places_left = no_capacity ? 0 : 10 - staged};
    return TFX_OK;
}
static tfx_status stop(tfx_context c, int32_t line, int32_t index, tfx_line_stop *out) {
    BEGIN(out);
    *out = (tfx_line_stop){.struct_size = sizeof(*out), .line = line, .stop_index = index,
        .stop_count = 6, .station_group = 1000 + index, .station_entity = 2000 + index,
        .station_index = 0, .terminal = 1, .passenger_load_enabled = 1,
        .passenger_max_load_bits = 0x3f800000};
    return TFX_OK;
}
static tfx_status loading(tfx_context c, int32_t line, int32_t index, tfx_stop_vehicles *out) {
    BEGIN(out);
    *out = (tfx_stop_vehicles){.struct_size = sizeof(*out), .line = line,
        .stop_index = index, .count = 1, .vehicles = {99}}; return TFX_OK;
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
    CHECK(c == 7 && staged < 5);
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
        .status = fail_result ? TFX_INVALID_STATE : TFX_OK}; return TFX_OK;
}
static tfx_status journey(tfx_context c, int32_t id, tfx_person_journey *out) {
    BEGIN(out); ++journey_calls[id - 100]; if (query_error) return TFX_NOT_FOUND;
    unsigned index = 0;
    while (index < staged && requests[index].expected.person != id) ++index;
    CHECK(index < staged);
    unsigned bit = 1u << (world - 1);
    *out = (tfx_person_journey){.struct_size = sizeof(*out), .world_id = world, .person = id,
        .destination = wrong_destination ? 999 : 777, .line = 97743, .boarding_stop = 1,
        .alighting_stop = 2, .vehicle = wrong_vehicle ? 88 : 99,
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
    sequence = staged = no_capacity = wrong_vehicle = fail_result = query_error = wrong_destination = 0;
    changed_home = reassigned = bad_identity = baseline_unassigned = 0;
    memset(missing_worlds, 0, sizeof(missing_worlds)); memset(invalidated_worlds, 0, sizeof(invalidated_worlds));
    memset(life_error, 0, sizeof(life_error)); memset(lifecycle_calls, 0, sizeof(lifecycle_calls));
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
    for (unsigned n = 0; n < 100 && staged < count; ++n) {
        for (unsigned i = 0; i < staged; ++i) boarded[i] = 3;
        tick();
    }
    CHECK(staged == count);
}
#if TFX_RESERVE_LAST_FOR_LIFECYCLE
static void reserve_tests(void) {
    reset(); stage_until(4);
    for (unsigned i = 0; i < 4; ++i) boarded[i] = 3;
    ticks(20); CHECK(staged == 4 && !tfx_pilot_complete());
    for (unsigned i = 1; i < 4; ++i) arrived[i] = 3;
    ticks(8); CHECK(staged == 4); /* Arrivals alone do not release the reserved attempt. */
    missing_worlds[0] = 2; ticks(8); CHECK(staged == 4 && !tfx_pilot_complete());
    missing_worlds[0] = 3; ticks(10);
    CHECK(staged == 5 && !tfx_pilot_complete() && strstr(output, "pilot_tracking_resumed"));
    const char *resolved = strstr(output, "pilot_lifecycle_resolved person=100"), *fifth = strstr(output, "ordinal=5");
    CHECK(resolved && fifth && resolved < fifth);
    boarded[4] = 3; ticks(4); arrived[4] = 3; ticks(2);
    CHECK(tfx_pilot_complete() && strstr(output, "reason=all_outcomes_observed staged=5"));
    CHECK(strstr(output, "destination_world_mask=0 removed_world_mask=3 invalidated_world_mask=0 outcome=passenger_removed"));
    ticks(20); CHECK(staged == 5); /* Never refund the retired passenger's attempt. */
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
    CHECK(tfx_pilot_complete() && strstr(output, "passenger_identity_unresolved"));
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
    printf("PASS: consumer and real policy max fixture query allowance=%u/64.\n", max_queries);
    return 0;
}
