/* Regression network: dense rail queues, forty bus stations and 81 vehicles.
 * Exercises the real consumer/policy, with short independent boarding windows. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "candidate_probe.h"
#include "probe_limits.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL %d: %s\n", __LINE__, #x); exit(1); } } while (0)
#define FIRST 100
#define PEOPLE 241
#define TARGET 80000
#define SOURCE 501
#define SUBSTITUTE 902
static tfx_client client;
static uint64_t sequence;
static unsigned queries, peak, window, missing_target, staged, boarding_reads, scan_calls;
static unsigned seen[PEOPLE], duplicate_staging[PEOPLE], seen_vehicles[81];
static tfx_waiting_leg_change requests[TFX_PASSENGER_LIMIT];
static char output[262144];
#define BEGIN(out) do { CHECK(c == 7 && (out)->struct_size == sizeof(*(out))); ++queries; } while (0)
static void log_line(const char *s) { CHECK(strlen(output) + strlen(s) < sizeof(output)); strcat(output, s); }
static int32_t demand_line(int32_t person) {
    return person < 180 ? SOURCE : person == 180 ? SUBSTITUTE : 1000 + (person - 181) / 4;
}
static tfx_status registration(const tfx_client *p) { client = *p; return TFX_OK; }
static tfx_status observation(tfx_context c, tfx_observation *out) {
    BEGIN(out); out->dispatch_sequence = sequence; return TFX_OK;
}
static tfx_status scan(tfx_context c, int32_t first, tfx_waiting_page *out) {
    BEGIN(out); ++scan_calls;
    *out = (tfx_waiting_page){.struct_size = sizeof(*out), .next_entity = -1};
    /* One hundred empty mask pages precede a dense populated region. A sparse
     * world's sweep must not force us to skip most people on every dense page. */
    if (first < 100) { out->next_entity = first + 1; return TFX_OK; }
    unsigned i = (unsigned)first - 100;
    while (i < PEOPLE && out->count < TFX_WAITING_PAGE_SIZE) out->persons[out->count++] = FIRST + (int32_t)i++;
    if (i < PEOPLE) out->next_entity = 100 + (int32_t)i;
    return TFX_OK;
}
static tfx_status waiting(tfx_context c, int32_t person, tfx_waiting_leg *out) {
    BEGIN(out); CHECK(person >= FIRST && person < FIRST + PEOPLE); ++seen[person-FIRST];
    int32_t line = demand_line(person);
    *out = (tfx_waiting_leg){.struct_size = sizeof(*out), .person = person, .line = line,
        .boarding_stop = line < 1000 ? 5 : 0, .alighting_stop = line < 1000 ? 0 : 1,
        .vehicle = -1, .destination = 777}; return TFX_OK;
}
static tfx_status stop(tfx_context c, int32_t line, int32_t index, tfx_line_stop *out) {
    BEGIN(out); CHECK(index >= 0 && index < 6);
    int32_t group = line < 1000 ? (index == 5 ? 1001 : index == 0 ? 1002 : 1003 + index) :
        10000 + (line - 1000) * 10 + index;
    *out = (tfx_line_stop){.struct_size = sizeof(*out), .line = line, .stop_index = index,
        .stop_count = 6, .station_group = group, .station_entity = group + 1000,
        .station_index = 0, .terminal = line == SUBSTITUTE ? 2 : 0,
        .passenger_load_enabled = 1, .passenger_max_load_bits = 0x3f800000}; return TFX_OK;
}
static tfx_status terminals(tfx_context c, int32_t line, int32_t index, tfx_stop_terminals *out) {
    CHECK(out->struct_size == sizeof(*out)); tfx_line_stop s = {.struct_size = sizeof(s)};
    CHECK(stop(c, line, index, &s) == TFX_OK);
    *out = (tfx_stop_terminals){.struct_size = sizeof(*out), .count = 1, .stop = s};
    out->terminals[0] = (tfx_stop_terminal){0, s.station_entity, s.terminal}; return TFX_OK;
}
static tfx_status loading(tfx_context c, int32_t line, int32_t index, tfx_stop_vehicles *out) {
    BEGIN(out); *out = (tfx_stop_vehicles){.struct_size = sizeof(*out), .line = line, .stop_index = index};
    if (line == SUBSTITUTE && index == (window ? 5 : 0) && !missing_target) {
        out->count = 1; out->vehicles[0] = TARGET;
    } else if (line >= 1000 && index == 0) {
        out->count = 2; out->vehicles[0] = 50000 + (line - 1000) * 2; out->vehicles[1] = out->vehicles[0] + 1;
    }
    return TFX_OK;
}
static tfx_status vehicle(tfx_context c, int32_t id, tfx_vehicle *out) {
    BEGIN(out); CHECK(id == TARGET || (id >= 50000 && id < 50080));
    ++seen_vehicles[id == TARGET ? 80 : (unsigned)(id - 50000)];
    if (id == TARGET && missing_target) return TFX_NOT_FOUND;
    if (id == TARGET && window) ++boarding_reads;
    *out = (tfx_vehicle){.struct_size = sizeof(*out), .vehicle = id,
        .line = id == TARGET ? SUBSTITUTE : 1000 + (id - 50000) / 2,
        .stop_index = id == TARGET && window ? 5 : 0, .state = 2,
        .load_state = id == TARGET && window ? 3 : 4, .arrival_locked = 1,
        .arrival_station_index = 0, .arrival_terminal = id == TARGET ? 2 : 0,
        .passenger_capacity = 100, .passenger_places_left = 100 - staged, .passengers_onboard = staged}; return TFX_OK;
}
static tfx_status platforms(tfx_context c, int32_t person, tfx_person_platforms *out) {
    BEGIN(out); *out = (tfx_person_platforms){.struct_size = sizeof(*out), .person = person,
        .count = 1, .platforms = {{2001, 0}}}; return TFX_OK;
}
static tfx_status terminal(tfx_context c, int32_t station, int32_t index, tfx_terminal *out) {
    BEGIN(out); *out = (tfx_terminal){.struct_size = sizeof(*out), .station_entity = station,
        .terminal = index, .terminal_count = 3, .passengers_load = 1, .passengers_unload = 1}; return TFX_OK;
}
static tfx_status presence(tfx_context c, int32_t id, tfx_entity_presence *out) {
    BEGIN(out); CHECK(id == 777); *out = (tfx_entity_presence){.struct_size = sizeof(*out), .entity = id, .has_components = 1}; return TFX_OK;
}
static tfx_status lifecycle(tfx_context c, int32_t id, tfx_person_lifecycle *out) {
    BEGIN(out); *out = (tfx_person_lifecycle){.struct_size = sizeof(*out), .person = id,
        .world_id = (unsigned)((sequence-1)%2)+1, .flags = 3, .target = 777, .destinations = {555, -1, 777}}; return TFX_OK;
}
static tfx_status request(tfx_context c, const tfx_waiting_leg_change *r, uint64_t *ticket) {
    CHECK(c == 7 && staged < TFX_PASSENGER_LIMIT && window && !missing_target);
    CHECK(r->expected.person >= 100 && r->expected.person < 180);
    CHECK(!duplicate_staging[r->expected.person-FIRST]++);
    CHECK(r->candidate_vehicle == TARGET && r->line == SUBSTITUTE && r->boarding_stop == 5 && r->alighting_stop == 0);
    queries += 2; CHECK(r->revalidate(c, r) == TFX_OK); queries += 5;
    requests[staged++] = *r; *ticket = staged; return TFX_OK;
}
static tfx_status result(tfx_context c, uint64_t ticket, tfx_change_result *out) {
    BEGIN(out); CHECK(ticket && ticket <= staged);
    *out = (tfx_change_result){.struct_size = sizeof(*out), .ticket = ticket,
        .person = requests[ticket-1].expected.person, .status = TFX_OK, .engine_attempted = 1}; return TFX_OK;
}
static tfx_status journey(tfx_context c, int32_t person, tfx_person_journey *out) {
    BEGIN(out); *out = (tfx_person_journey){.struct_size = sizeof(*out), .person = person,
        .world_id = (unsigned)((sequence-1)%2)+1, .destination = 777, .line = SUBSTITUTE,
        .boarding_stop = 5, .alighting_stop = 0, .vehicle = TARGET, .flags = TFX_JOURNEY_ONBOARD}; return TFX_OK;
}
tfx_status tfx_get_api(uint32_t version, tfx_api *base) {
    CHECK(version == 8 || version == 7); base->abi_version = version;
    base->capabilities = version == 8 ? 511 : 255;
    base->register_client = registration; base->read_observation = observation;
    tfx_api_v5 *api = (tfx_api_v5 *)base;
    api->v4.v3.v2.read_waiting_leg = waiting; api->v4.v3.v2.scan_waiting_people = scan;
    api->v4.v3.v2.read_line_stop = stop; api->v4.v3.read_vehicle = vehicle;
    api->v4.v3.read_stop_vehicles = loading; api->v4.read_person_platforms = platforms;
    api->read_terminal = terminal; api->read_entity_presence = presence;
    if (version == 8) ((tfx_api_v8 *)base)->read_stop_terminals = terminals;
    ((tfx_api_v7 *)base)->read_person_lifecycle = lifecycle;
    ((tfx_api_v6 *)base)->request_waiting_leg_change = request;
    ((tfx_api_v6 *)base)->read_change_result = result; ((tfx_api_v6 *)base)->read_person_journey = journey;
    return TFX_OK;
}
static void reset(void) {
    sequence = window = missing_target = staged = boarding_reads = scan_calls = 0;
    memset(seen_vehicles, 0, sizeof(seen_vehicles));
    memset(seen, 0, sizeof(seen)); memset(duplicate_staging, 0, sizeof(duplicate_staging)); output[0] = 0;
    CHECK(tfx_register_candidate_probe(log_line) == TFX_OK);
}
static void tick(void) {
    ++sequence; queries = 0; client.before_terminal_selection(7, NULL);
    if (queries > peak) peak = queries; CHECK(queries <= 64);
}
static void ticks(unsigned n) { while (n--) tick(); }
static void warm(unsigned phase) {
    reset(); ticks(600 + phase); CHECK(!staged);
    for (unsigned i = 0; i < PEOPLE; ++i) CHECK(seen[i]);
    for (unsigned i = 0; i < 81; ++i) CHECK(seen_vehicles[i]);
    CHECK(scan_calls < 350); /* Dense pages are drained, not rescanned per person. */
}
int main(void) {
    for (unsigned phase = 0; phase < 16; ++phase) {
        warm(phase); window = 1; ticks(16);
        if (!staged) fprintf(stderr, "phase=%u boarding_reads=%u\n%s", phase, boarding_reads, output);
        CHECK(staged > 0 && strstr(output, "cross_platform=1"));
        CHECK(!strstr(output, "candidate_query_error"));
    }
    warm(0); window = 1; ticks(120);
    CHECK(staged >= 12); /* Revisit the arrival and consume several distinct hints. */
    CHECK(strstr(output, "pilot_boarded_both"));
    /* A priority hint must never authorize use of a closed loading window or a
     * deleted vehicle. Find a phase where lookahead saw it before evaluation. */
    unsigned stale_cases = 0;
    for (unsigned phase = 0; phase < 32 && stale_cases < 2; ++phase) {
        warm(phase); window = 1; tick();
        if (staged || !boarding_reads) continue;
        window = 0; missing_target = stale_cases;
        ticks(16); CHECK(!staged && !strstr(output, "candidate_query_error")); ++stale_cases;
    }
    CHECK(stale_cases == 2);
    printf("PASS: all 241 waiters learned across sparse pages; 80-person rail queue survives 40 bus stations/81 vehicles; 16 boarding-window phases, repeated switches and stale/deleted priority hints; peak=%u/64 queries.\n", peak);
    return 0;
}
