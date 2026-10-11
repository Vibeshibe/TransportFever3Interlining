/* Host scheduling tests with copied-query/writer doubles. Real component mutation
 * and native call ordering remain covered by query_tests. Run in fresh processes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "host.h"
#include "probe_limits.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL %d: %s\n", __LINE__, #x); exit(1); } } while (0)
static tfx_api_v6 api = {.v5.v4.v3.v2.base.struct_size = sizeof(api)};
static uint64_t tickets[2];
static int32_t people[2];
static unsigned count, writes, validations, readback, fail_write, reject_policy;
static tfx_status expected_request, expected_results[2];
static unsigned expected_attempts[2];
static tfx_context stale;
static uint64_t expired_ticket;
static uint64_t arrival_bits;
static unsigned journey_mode, journey_wrong;

tfx_status tfx_query_waiting(tfx_reader *r, int32_t person, tfx_waiting_leg *out) {
    (void)r;
    *out = (tfx_waiting_leg){.struct_size = sizeof(*out), .person = person,
        .line = 10, .boarding_stop = 1, .alighting_stop = 2, .vehicle = -1, .destination = 99,
        .arrival_time_bits = arrival_bits};
    return TFX_OK;
}
tfx_status tfx_query_waiting_raw(tfx_reader *r, int32_t p, unsigned char out[32]) {
    (void)r; (void)p; memset(out, 0xa5, 32); return TFX_OK;
}
#define STUB1(name, type) tfx_status name(tfx_reader *r, int32_t a, type *out) { \
    (void)r; (void)a; (void)out; return TFX_QUERY_UNAVAILABLE; }
#define STUB2(name, type) tfx_status name(tfx_reader *r, int32_t a, int32_t b, type *out) { \
    (void)r; (void)a; (void)b; (void)out; return TFX_QUERY_UNAVAILABLE; }
STUB1(tfx_query_scan, tfx_waiting_page)
STUB1(tfx_query_scan_lines, tfx_line_page)
STUB1(tfx_query_vehicle, tfx_vehicle)
STUB1(tfx_query_platforms, tfx_person_platforms)
STUB1(tfx_query_presence, tfx_entity_presence)
tfx_status tfx_query_journey(tfx_reader *r, int32_t person, tfx_person_journey *out) {
    if (!journey_mode) return TFX_QUERY_UNAVAILABLE;
    *out = (tfx_person_journey){.struct_size = sizeof(*out), .world_id = r->world_id,
        .person = person, .destination = 99, .line = 20, .vehicle = journey_wrong ? 51 : 50,
        .boarding_stop = 1, .alighting_stop = 2, .flags = TFX_JOURNEY_ONBOARD};
    return TFX_OK;
}
STUB1(tfx_query_lifecycle, tfx_person_lifecycle)
STUB2(tfx_query_stop, tfx_line_stop)
STUB2(tfx_query_stop_terminals, tfx_stop_terminals)
STUB2(tfx_query_stop_vehicles, tfx_stop_vehicles)
STUB2(tfx_query_terminal, tfx_terminal)

static tfx_status validate(tfx_context context, const tfx_waiting_leg_change *request) {
    tfx_waiting_leg leg = {.struct_size = sizeof(leg)};
    CHECK(api.v5.v4.v3.v2.read_waiting_leg(stale, request->expected.person, &leg) == TFX_STALE_CONTEXT);
    CHECK(api.v5.v4.v3.v2.read_waiting_leg(context, request->expected.person, &leg) == TFX_OK);
    ++validations;
    return reject_policy ? TFX_POLICY_REJECTED : TFX_OK;
}
static tfx_status write_change(tfx_reader *r, const tfx_waiting_leg_change *q,
                             const unsigned char raw[32], uint32_t *attempted, void *user) {
    (void)r; (void)q; (void)user;
    CHECK(raw[31] == 0xa5);
    ++writes; *attempted = 1;
    return fail_write ? TFX_INVALID_STATE : TFX_OK;
}
static void client(tfx_context context, void *unused) {
    (void)unused;
    if (journey_mode) {
        tfx_person_journey j = {.struct_size = sizeof(j)};
        CHECK(api.read_person_journey(context, people[0], &j) == TFX_OK); return;
    }
    if (readback && expired_ticket) {
        tfx_change_result result = {.struct_size = sizeof(result), .ticket = 12345};
        tfx_change_result unchanged = result;
        CHECK(api.read_change_result(context, expired_ticket, &result) == TFX_NOT_FOUND);
        CHECK(!memcmp(&result, &unchanged, sizeof(result)));
    }
    for (unsigned i = 0; i < count; ++i) {
        if (readback) {
            tfx_change_result result = {.struct_size = sizeof(result)};
            CHECK(api.read_change_result(context, tickets[i], &result) == TFX_OK);
            CHECK(result.status == expected_results[i] && result.engine_attempted == expected_attempts[i]);
        } else {
            tfx_observation obs = {.struct_size = sizeof(obs)};
            CHECK(api.v5.v4.v3.v2.base.read_observation(context, &obs) == TFX_OK);
            tfx_waiting_leg_change q = {.struct_size = sizeof(q), .dispatch_sequence = obs.dispatch_sequence,
                .line = 20, .boarding_stop = 1, .alighting_stop = 2, .candidate_vehicle = 50, .revalidate = validate};
            tfx_query_waiting(NULL, people[i], &q.expected);
            stale = context;
            uint64_t ticket = 999;
            CHECK(api.request_waiting_leg_change(context, &q, &ticket) == expected_request);
            if (expected_request == TFX_OK) tickets[i] = ticket;
            else CHECK(ticket == 999);
        }
    }
}
static void dispatch(unsigned world, unsigned get_results) {
    tfx_reader reader = {.world_id = world}; readback = get_results;
    CHECK(tfx_dispatch_with_writer(0, &reader, write_change, NULL) == TFX_OK);
}
int main(int argc, char **argv) {
    CHECK(argc <= 2);
    CHECK(tfx_get_api(6, &api.v5.v4.v3.v2.base) == TFX_OK);
    tfx_client c = {.struct_size = sizeof(c), .id = "tests.batch", .before_terminal_selection = client};
    CHECK(api.v5.v4.v3.v2.base.register_client(&c) == TFX_OK);
    if (argc == 2 && !strcmp(argv[1], "history")) {
        count = 1; people[0] = 1; expected_request = TFX_OK;
        reject_policy = 1; expected_results[0] = TFX_POLICY_REJECTED;
        uint64_t oldest = 0;
        for (unsigned i = 0; i < 80; ++i) {
            dispatch(i % 2 + 1, 0);
            if (!i) oldest = tickets[0];
            dispatch(1, 1);
        }
        expired_ticket = oldest; dispatch(1, 1);
        CHECK(writes == 0 && validations == 80);
        /* Pending requests spanning ring wrap retain distinct conflict results. */
        count = 2; people[1] = people[0];
        expected_results[0] = expected_results[1] = TFX_REQUEST_CONFLICT;
        for (unsigned i = 0; i < 40; ++i) { dispatch(1, 0); dispatch(2, 1); }
        CHECK(writes == 0 && validations == 80);
        puts("PASS: stale history tickets preserve outputs; pending conflicts survive result-ring wrap without engine attempts.");
        return 0;
    }
    count = 2; people[0] = people[1] = 1; expected_request = TFX_OK;
    expected_results[0] = expected_results[1] = TFX_REQUEST_CONFLICT;
    dispatch(1, 0); dispatch(1, 1); CHECK(writes == 0 && validations == 0);
    count = 1; reject_policy = 1; expected_results[0] = TFX_POLICY_REJECTED;
    dispatch(1, 0); dispatch(1, 1); CHECK(writes == 0 && validations == 1);
    reject_policy = 0;
    if (argc == 2) {
        CHECK(!strcmp(argv[1], "failure")); fail_write = 1;
        expected_results[0] = TFX_INVALID_STATE; expected_attempts[0] = 1;
        dispatch(1, 0); dispatch(1, 1); CHECK(writes == 1);
        people[0] = 2; expected_request = TFX_LIMIT_REACHED;
        dispatch(2, 0); CHECK(writes == 1 && validations == 2);
        puts("PASS: a failed postcondition latches all later passenger mutations shut.");
        return 0;
    }
    count = 2; people[1] = 2;
    expected_results[0] = TFX_OK; expected_attempts[0] = 1;
    expected_results[1] = TFX_LIMIT_REACHED;
    dispatch(1, 0); dispatch(1, 1); CHECK(writes == 1 && validations == 2);
    count = 1; expected_request = TFX_LIMIT_REACHED;
    dispatch(2, 0); CHECK(writes == 1); /* Same person, stale replica state. */
    CHECK(api.v5.v4.v3.v2.base.unregister_client(c.id) == TFX_OK);
    strcpy(c.id, "tests.other-client");
    CHECK(api.v5.v4.v3.v2.base.register_client(&c) == TFX_OK);
    dispatch(1, 0); CHECK(writes == 1); /* Allowance is not client-owned. */
    expected_request = TFX_OK;
    for (people[0] = 2; people[0] <= TFX_PASSENGER_LIMIT; ++people[0]) {
        dispatch((unsigned)people[0] % 2 + 1, 0); dispatch(1, 1);
    }
    CHECK(writes == TFX_PASSENGER_LIMIT && validations == TFX_PASSENGER_LIMIT + 1);
    expected_request = TFX_LIMIT_REACHED; dispatch(1, 0); CHECK(writes == TFX_PASSENGER_LIMIT);
    printf("PASS: %u distinct people, cross-world/client deduplication, one attempt per dispatch, fresh validation, conflicts and final cap.\n", (unsigned)TFX_PASSENGER_LIMIT);
    return 0;
}
