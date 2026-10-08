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
    BEGIN(out); if (query_error) return TFX_NOT_FOUND;
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
tfx_status tfx_get_api(uint32_t version, tfx_api *base) {
    CHECK(version == 5 || version == 6);
    base->abi_version = version; base->capabilities = version == 5 ? 31 : 127;
    base->register_client = registration; base->read_observation = observation;
    tfx_api_v5 *api = (tfx_api_v5 *)base;
    api->v4.v3.v2.read_waiting_leg = waiting; api->v4.v3.v2.scan_waiting_people = scan;
    api->v4.v3.v2.read_line_stop = stop; api->v4.v3.read_vehicle = vehicle;
    api->v4.v3.read_stop_vehicles = loading; api->v4.read_person_platforms = platforms;
    api->read_terminal = terminal; api->read_entity_presence = presence;
    if (version == 6) {
        tfx_api_v6 *changes = (tfx_api_v6 *)base;
        changes->request_waiting_leg_change = request; changes->read_change_result = result;
        changes->read_person_journey = journey;
    }
    return TFX_OK;
}
static void reset(void) {
    sequence = staged = no_capacity = wrong_vehicle = fail_result = query_error = wrong_destination = 0;
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
int main(void) {
    reset(); tick(); CHECK(staged == 1);
    ticks(20); CHECK(staged == 1); /* No boarding yet. */
    boarded[0] = 1; ticks(10); CHECK(staged == 1); /* One replica is insufficient. */
    for (unsigned i = 0; i < 5; ++i) {
        CHECK(staged == i + 1);
        boarded[i] = 3;
        ticks(10);
        CHECK(staged == (i < 4 ? i + 2 : 5));
    }
    ticks(10); CHECK(staged == 5 && !tfx_pilot_complete());
    for (unsigned i = 0; i < 5; ++i) arrived[i] = 3;
    ticks(2); CHECK(tfx_pilot_complete() && strstr(output, "reason=all_destinations_observed staged=5"));
    reset(); tick(); boarded[0] = 3; no_capacity = 1;
    ticks(20); CHECK(staged == 1 && strstr(output, "reason=no_capacity"));
    no_capacity = 0; ticks(10); CHECK(staged == 2);
    reset(); tick(); boarded[0] = 3; wrong_vehicle = 1; tick();
    CHECK(tfx_pilot_complete() && staged == 1 && strstr(output, "unexpected_first_boarding"));
    reset(); tick(); fail_result = 1; tick();
    CHECK(tfx_pilot_complete() && strstr(output, "change_rejected_or_failed"));
    reset(); tick(); wrong_destination = 1; tick();
    CHECK(tfx_pilot_complete() && strstr(output, "destination_changed_before_arrival"));
    reset(); tick(); query_error = 1; ticks(8);
    CHECK(tfx_pilot_complete() && staged == 1 && strstr(output, "reason=query_errors"));
    reset(); tick(); sequence = TFX_OBSERVATION_LIMIT - 1; tick();
    CHECK(tfx_pilot_complete() && strstr(output, "reason=callback_limit") &&
        strstr(output, "onboard_world_mask=0 destination_world_mask=0"));
    reset(); sequence = TFX_OBSERVATION_LIMIT - 1; tick();
    CHECK(tfx_pilot_complete() && staged == 0);
    printf("PASS: real policy/consumer with five distinct passengers, delayed replica boarding, fresh capacity, failed changes, destination guards, final cap and tracking; max fixture query allowance=%u/64.\n", max_queries);
    return 0;
}
