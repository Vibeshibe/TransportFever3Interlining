#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/interlining/native_candidate.h"
#include "pilot_cache.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL %d: %s\n", __LINE__, #x); exit(1); } } while (0)
static tfx_waiting_leg leg;
static tfx_vehicle vehicle;
static tfx_line_stop stops[16];
static unsigned stop_count, query_count, fail_query;
static tfx_person_platforms platforms;
static tfx_terminal terminals[2];
static uint32_t present, indexed;
static tfx_status forced;
#define BEGIN(out) do { CHECK(c == 7 && (out)->struct_size == sizeof(*(out))); ++query_count; if (forced || (fail_query && query_count == fail_query)) return forced ? forced : TFX_NOT_FOUND; } while (0)
static tfx_status observation(tfx_context c, tfx_observation *out) {
    BEGIN(out); *out = (tfx_observation){sizeof(*out), 0x3e4ccccd, 123}; return TFX_OK;
}
static tfx_status waiting(tfx_context c, int32_t id, tfx_waiting_leg *out) {
    BEGIN(out); CHECK(id == 42); *out = leg; return TFX_OK;
}
static tfx_status read_vehicle(tfx_context c, int32_t id, tfx_vehicle *out) {
    BEGIN(out); CHECK(id == 99); *out = vehicle; return TFX_OK;
}
static tfx_status read_stop(tfx_context c, int32_t line, int32_t stop, tfx_line_stop *out) {
    BEGIN(out);
    for (unsigned i = 0; i < stop_count; ++i) if (stops[i].line == line && stops[i].stop_index == stop) {
        *out = stops[i]; return TFX_OK;
    }
    return TFX_NOT_FOUND;
}
static tfx_status read_platforms(tfx_context c, int32_t id, tfx_person_platforms *out) {
    BEGIN(out); CHECK(id == 42); *out = platforms; return TFX_OK;
}
static tfx_status read_terminal(tfx_context c, int32_t station, int32_t terminal, tfx_terminal *out) {
    BEGIN(out);
    for (unsigned i = 0; i < 2; ++i) if (terminals[i].station_entity == station && terminals[i].terminal == terminal) {
        *out = terminals[i]; return TFX_OK;
    }
    return TFX_NOT_FOUND;
}
static tfx_status presence(tfx_context c, int32_t id, tfx_entity_presence *out) {
    BEGIN(out); CHECK(id == 777); *out = (tfx_entity_presence){sizeof(*out), id, present, 0}; return TFX_OK;
}
static tfx_status loading(tfx_context c, int32_t line, int32_t stop, tfx_stop_vehicles *out) {
    BEGIN(out); CHECK(line == 20 && stop == 2);
    *out = (tfx_stop_vehicles){.struct_size = sizeof(*out), .line = line, .stop_index = stop,
        .count = indexed, .vehicles = {99}}; return TFX_OK;
}
static tfx_api_v5 api;
static void fixture(void) {
    forced = TFX_OK; present = indexed = 1;
    stop_count = 4; query_count = fail_query = 0;
    leg = (tfx_waiting_leg){.struct_size = sizeof(leg), .person = 42, .line = 10, .boarding_stop = 1,
        .alighting_stop = 2, .vehicle = -1, .destination = 777, .flags = TFX_HAS_TERMINAL_PLACE,
        .arrival_time_bits = UINT64_C(0x123456789abcdef0)};
    vehicle = (tfx_vehicle){.struct_size = sizeof(vehicle), .vehicle = 99, .line = 20, .stop_index = 2,
        .state = 2, .load_state = 3, .arrival_station_index = 0, .arrival_terminal = 1, .arrival_locked = 1,
        .passenger_capacity = 40, .passengers_onboard = 35, .passengers_reserved = 2, .passenger_places_left = 3};
    for (unsigned i = 0; i < 4; ++i) stops[i] = (tfx_line_stop){.struct_size = sizeof(stops[i]),
        .line = i < 2 ? 10 : 20, .stop_index = (int32_t)(i % 2) + (i < 2 ? 1 : 2), .stop_count = 6,
        .station_group = i % 2 ? 2000 : 1000, .station_index = 0,
        .station_entity = i % 2 ? 200 : 100, .terminal = i % 2 ? 0 : 1,
        .passenger_load_enabled = 1, .passenger_max_load_bits = 0x3f800000};
    platforms = (tfx_person_platforms){.struct_size = sizeof(platforms), .person = 42, .count = 1,
        .platforms = {{100, 1}}};
    terminals[0] = (tfx_terminal){sizeof(tfx_terminal), 100, 1, 3, 1, 0};
    terminals[1] = (tfx_terminal){sizeof(tfx_terminal), 200, 0, 3, 0, 1};
    api = (tfx_api_v5){0};
    api.v4.v3.v2.base.struct_size = sizeof(api); api.v4.v3.v2.base.abi_version = 5;
    api.v4.v3.v2.base.capabilities = 31; api.v4.v3.v2.base.read_observation = observation;
    api.v4.v3.v2.read_waiting_leg = waiting; api.v4.v3.v2.read_line_stop = read_stop;
    api.v4.v3.read_vehicle = read_vehicle; api.v4.v3.read_stop_vehicles = loading;
    api.v4.read_person_platforms = read_platforms; api.read_terminal = read_terminal;
    api.read_entity_presence = presence;
}
static void expect(it_reason reason) {
    it_candidate result;
    CHECK(it_validate_candidate(&api, 7, 42, 99, &result) == TFX_OK);
    CHECK(result.reason == reason && result.requires_engine_revalidation == 1);
}
static void route_fixture(int hops) {
    fixture();
    tfx_line_stop old = stops[0], replacement = stops[2];
    leg.alighting_stop = leg.boarding_stop + hops;
    stop_count = 0;
    for (int route = 0; route < 2; ++route) {
        for (int i = 0; i <= hops; ++i) {
            tfx_line_stop stop = route ? replacement : old;
            stop.stop_index += i;
            /* Both routes have consistent counts, including unused later stops. */
            stop.stop_count = route ? 8 : 7;
            stop.station_group = i == hops ? 2000 : 1000 + i;
            stop.station_entity = i == hops ? 200 : 100 + i;
            stop.terminal = i == hops ? 0 : 1;
            stops[stop_count++] = stop;
        }
    }
}
static void route_tests(void) {
    it_candidate result;
    for (int hops = 1; hops <= IT_MAX_LEG_HOPS; ++hops) {
        route_fixture(hops);
        CHECK(it_validate_candidate(&api, 7, 42, 99, &result) == TFX_OK && result.reason == IT_CANDIDATE);
        CHECK(result.alighting_stop == 2 + hops && result.expected.alighting_stop == 1 + hops);
        CHECK(query_count <= 18);
        /* An express reaches the same endpoint with fewer stop intervals. */
        stops[hops + 2] = stops[2 * hops + 1]; stops[hops + 2].stop_index = 3;
        expect(IT_CANDIDATE);
        CHECK(it_validate_candidate(&api, 7, 42, 99, &result) == TFX_OK && result.alighting_stop == 3);
    }
    route_fixture(4); stops[6] = stops[7]; stops[6].stop_index = 3;
    stops[7] = stops[9]; stops[7].stop_index = 4; /* A,C,E subsequence of A,B,C,D,E. */
    CHECK(it_validate_candidate(&api, 7, 42, 99, &result) == TFX_OK && result.reason == IT_CANDIDATE && result.alighting_stop == 4);
    route_fixture(3); stops[5].station_group = 9000; expect(IT_ROUTE_DIVERGENCE); /* Branch/detour. */
    route_fixture(3); stops[5].station_group = 1002; stops[6].station_group = 1001;
    expect(IT_ROUTE_DIVERGENCE); /* Same stations in reverse order. */
    route_fixture(3); stops[5].station_group = 1000; expect(IT_ROUTE_DIVERGENCE); /* Returns to boarding. */
    route_fixture(3); stops[2].station_group = stops[1].station_group; expect(IT_UNSUPPORTED_LEG);
    route_fixture(3); stops[3].station_group = stops[0].station_group; expect(IT_UNSUPPORTED_LEG);
    route_fixture(3); stops[5].station_group = 2000; stops[5].terminal = 9;
    expect(IT_ALIGHTING_MISMATCH); /* Do not skip wrong-platform first visit for later matching visit. */
    route_fixture(3); stops[7].has_alternative_terminals = 1; expect(IT_ALTERNATIVE_TERMINALS);
    route_fixture(3); for (unsigned i = 4; i < 8; ++i) stops[i].stop_count = 4;
    expect(IT_ALIGHTING_MISMATCH); /* End of route; never wrap. */
    route_fixture(3); stops[0].stop_count = leg.alighting_stop; expect(IT_UNSUPPORTED_LEG);
    route_fixture(3); leg.alighting_stop = 0; expect(IT_UNSUPPORTED_LEG);
    route_fixture(3); leg.boarding_stop = -1; expect(IT_UNSUPPORTED_LEG);
    route_fixture(3); stops[1].stop_count--;
    CHECK(it_validate_candidate(&api, 7, 42, 99, &result) == TFX_INVALID_STATE);
    route_fixture(3); stops[6].stop_count--;
    CHECK(it_validate_candidate(&api, 7, 42, 99, &result) == TFX_INVALID_STATE);
    /* Failure at every query, including the last one, preserves caller output. */
    route_fixture(4);
    CHECK(it_validate_candidate(&api, 7, 42, 99, &result) == TFX_OK);
    unsigned total = query_count;
    for (unsigned n = 1; n <= total; ++n) {
        route_fixture(4); fail_query = n;
        memset(&result, 0xa5, sizeof(result)); it_candidate before = result;
        CHECK(it_validate_candidate(&api, 7, 42, 99, &result) == TFX_NOT_FOUND);
        CHECK(!memcmp(&result, &before, sizeof(result)));
    }
    puts("PASS: bounded multi-stop/express routing, original endpoint, ordered stations, branches/loops/platform exclusions and late-read failure preservation.");
}
int main(void) {
    /* Regression: passengers discovered after the first full batch must remain
     * observable, while remembering existing IDs must not churn the cache. */
    int32_t ids[TFX_PILOT_PEOPLE] = {0}; uint32_t count = 0, cursor = 0;
    for (int32_t id = 0; id < 96; ++id) {
        int32_t slot = tfx_pilot_remember(ids, &count, &cursor, id);
        CHECK(slot >= 0 && slot < 32 && ids[slot] == id && count <= 32);
        uint32_t before = cursor;
        CHECK(tfx_pilot_remember(ids, &count, &cursor, id) == -1 && cursor == before);
        for (uint32_t a = 0; a < count; ++a)
            for (uint32_t b = a + 1; b < count; ++b) CHECK(ids[a] != ids[b]);
    }
    CHECK(count == 32);
    puts("PASS: bounded discovery admits later passengers and preserves existing IDs without duplicate slots.");
    int32_t small[16]; count = cursor = 0;
    for (int32_t id = 0; id < 48; ++id)
        CHECK(tfx_pilot_remember_bounded(small, &count, &cursor, 16, id) >= 0);
    CHECK(count == 16);
    for (unsigned i = 0; i < count; ++i) CHECK(small[i] >= 32);
    small[7] = -1; uint32_t before_cursor = cursor;
    CHECK(tfx_pilot_remember_bounded(small, &count, &cursor, 16, 99) == 7 && cursor == before_cursor);
    CHECK(tfx_pilot_remember_bounded(small, &count, &cursor, 16, 99) == -1);
    route_tests();
    fixture(); it_candidate result;
    CHECK(it_validate_candidate(&api, 7, 42, 99, &result) == TFX_OK && result.reason == IT_CANDIDATE);
    CHECK(result.dispatch_sequence == 123 && result.proposed_line == 20 && result.boarding_stop == 2 && result.alighting_stop == 3);
    CHECK(result.observed_places_left == 3 && result.boarding_station == 100 && result.alighting_station == 200);
    CHECK(!memcmp(&result.expected, &leg, sizeof(leg))); /* Preserve destination, age and waiting place. */
    fixture(); leg.vehicle = -2; expect(IT_UNSUPPORTED_WAITING_STATE);
    fixture(); leg.vehicle = -3; expect(IT_UNSUPPORTED_WAITING_STATE);
    fixture(); leg.vehicle = 99; expect(IT_ALREADY_ASSIGNED);
    fixture(); leg.alighting_stop = 6; expect(IT_UNSUPPORTED_LEG);
    fixture(); leg.boarding_stop = INT32_MAX; expect(IT_UNSUPPORTED_LEG);
    fixture(); vehicle.line = 10; expect(IT_SAME_LINE);
    fixture(); vehicle.load_state = 5; expect(IT_VEHICLE_NOT_LOADING);
    fixture(); vehicle.user_stopped = 1; expect(IT_VEHICLE_NOT_LOADING);
    fixture(); stops[2].stop_count = 3; expect(IT_UNSUPPORTED_LEG);
    fixture(); stops[3].has_alternative_terminals = 1; expect(IT_ALTERNATIVE_TERMINALS);
    fixture(); stops[2].terminal = 2; expect(IT_BOARDING_MISMATCH);
    fixture(); stops[3].station_group = 3000; expect(IT_ALIGHTING_MISMATCH);
    fixture(); vehicle.arrival_station_index = 100; expect(IT_ACTUAL_TERMINAL_MISMATCH);
    fixture(); vehicle.arrival_locked = 0; expect(IT_ACTUAL_TERMINAL_MISMATCH);
    fixture(); stops[2].passenger_load_enabled = 0; expect(IT_LOADING_DISABLED);
    fixture(); stops[2].passenger_max_load_bits = 0x3f000000; expect(IT_UNSUPPORTED_LOAD_LIMIT);
    fixture(); vehicle.passengers_reserved = 5; vehicle.passenger_places_left = 0; expect(IT_NO_CAPACITY);
    fixture(); vehicle.next_passenger_load_seconds_bits = 0x3e4ccccd; expect(IT_LOAD_TIMER_PENDING);
    fixture(); vehicle.next_passenger_load_seconds_bits = 0x7fc00000; expect(IT_LOAD_TIMER_PENDING);
    fixture(); vehicle.next_passenger_load_seconds_bits = 0xbf000000; expect(IT_CANDIDATE);
    fixture(); platforms.platforms[0].terminal = 2; expect(IT_NO_PLATFORM_MEMBERSHIP);
    fixture(); platforms.count = 0; expect(IT_NO_PLATFORM_MEMBERSHIP);
    fixture(); terminals[0].passengers_load = 0; expect(IT_LOADING_DISABLED);
    fixture(); terminals[1].passengers_unload = 0; expect(IT_UNLOADING_DISABLED);
    fixture(); present = 0; expect(IT_DESTINATION_MISSING);
    fixture(); indexed = 0; expect(IT_NOT_IN_LOADING_INDEX);
    fixture(); forced = TFX_QUERY_BUDGET_EXHAUSTED;
    memset(&result, 0xa5, sizeof(result)); it_candidate unchanged = result;
    CHECK(it_validate_candidate(&api, 7, 42, 99, &result) == TFX_QUERY_BUDGET_EXHAUSTED);
    CHECK(!memcmp(&result, &unchanged, sizeof(result)));
    CHECK(it_validate_candidate(NULL, 7, 42, 99, &result) == TFX_INVALID_ARGUMENT);
    CHECK(!strcmp(it_reason_name(IT_CANDIDATE), "candidate"));
    puts("PASS: candidate route/platform/terminal/destination/capacity gates, preserved expected state, conservative exclusions and query failures.");
    return 0;
}
