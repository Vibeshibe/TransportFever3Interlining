/* Save-specific discovery and logging; candidate policy uses only the SDK. */
#include <stdio.h>
#include <string.h>
#include "candidate_probe.h"
#include "../src/interlining/native_candidate.h"
static tfx_api_v5 api = {.v4.v3.v2.base.struct_size = sizeof(api)};
static void (*log_line)(const char *);
static int32_t scan_cursor, people[32], vehicles[16];
static uint32_t person_count, vehicle_count, vehicle_cursor, person_cursor;
static uint32_t failures, attempts, accepted, logged, ready;
static uint32_t reasons[IT_REASON_COUNT];
static unsigned char last_reason[32][16]; /* Zero is unseen; reason+1 otherwise. */

static void error(const char *operation, tfx_status status) {
    if (failures++ >= 8) return;
    char text[160]; snprintf(text, sizeof(text), "candidate_query_error operation=%s status=%u\n", operation, status);
    log_line(text);
}
static void remember(int32_t *ids, uint32_t *count, uint32_t limit, int32_t id) {
    for (uint32_t i = 0; i < *count; ++i) if (ids[i] == id) return;
    if (*count < limit) ids[(*count)++] = id;
}
static void discover(tfx_context context, uint64_t sequence) {
    tfx_waiting_page page = {.struct_size = sizeof(page)};
    tfx_status status = api.v4.v3.v2.scan_waiting_people(context, scan_cursor, &page);
    if (status != TFX_OK) { error("scan", status); return; }
    scan_cursor = page.next_entity < 0 ? 0 : page.next_entity;
    for (uint32_t i = 0; i < page.count; ++i) remember(people, &person_count, 32, page.persons[i]);
    static const int32_t lines[3] = {98278, 97742, 97743};
    int32_t line = lines[(sequence - 1) % 3];
    tfx_line_stop stop = {.struct_size = sizeof(stop)};
    status = api.v4.v3.v2.read_line_stop(context, line, 0, &stop);
    if (status != TFX_OK) { error("line", status); return; }
    tfx_stop_vehicles list = {.struct_size = sizeof(list)};
    status = api.v4.v3.read_stop_vehicles(context, line, (int32_t)((sequence / 3) % (uint32_t)stop.stop_count), &list);
    if (status != TFX_OK) { error("stop_vehicles", status); return; }
    for (uint32_t i = 0; i < list.count; ++i) remember(vehicles, &vehicle_count, 16, list.vehicles[i]);
}
static void evaluate(tfx_context context, uint64_t sequence) {
    uint32_t checks = vehicle_count < 4 ? vehicle_count : 4;
    for (uint32_t i = 0; i < checks && failures < 8; ++i) {
        uint32_t vi = vehicle_cursor++ % vehicle_count;
        tfx_vehicle v = {.struct_size = sizeof(v)};
        tfx_status status = api.v4.v3.read_vehicle(context, vehicles[vi], &v);
        if (status == TFX_NOT_FOUND) continue;
        if (status != TFX_OK) { error("vehicle", status); return; }
        uint32_t timer = v.next_passenger_load_seconds_bits;
        if (v.state != 2 || v.load_state != 3 || v.line < 0 || v.stop_index < 0 ||
            (!(timer & 0x80000000u) && timer != 0)) continue;
        ++ready;
        tfx_line_stop arriving = {.struct_size = sizeof(arriving)};
        status = api.v4.v3.v2.read_line_stop(context, v.line, v.stop_index, &arriving);
        if (status != TFX_OK) { error("arriving_stop", status); return; }
        uint32_t samples = person_count < 8 ? person_count : 8, attempted = 0;
        for (uint32_t j = 0; j < samples && attempted < 2; ++j) {
            uint32_t pi = person_cursor++ % person_count;
            tfx_waiting_leg leg = {.struct_size = sizeof(leg)};
            status = api.v4.v3.v2.read_waiting_leg(context, people[pi], &leg);
            if (status == TFX_NOT_FOUND) continue;
            if (status != TFX_OK) { error("waiting", status); return; }
            if (leg.line == v.line || leg.vehicle != -1) continue;
            tfx_line_stop board = {.struct_size = sizeof(board)};
            status = api.v4.v3.v2.read_line_stop(context, leg.line, leg.boarding_stop, &board);
            if (status != TFX_OK) { error("waiting_stop", status); return; }
            if (board.station_group != arriving.station_group) continue;
            ++attempted;
            it_candidate proposal;
            status = it_validate_candidate(&api, context, leg.person, v.vehicle, &proposal);
            if (status != TFX_OK) { error("validate", status); return; }
            ++attempts; ++reasons[proposal.reason];
            if (proposal.reason == IT_CANDIDATE) ++accepted;
            if (logged >= 80 || last_reason[pi][vi] == (unsigned char)(proposal.reason + 1)) continue;
            last_reason[pi][vi] = (unsigned char)(proposal.reason + 1);
            char text[768];
            snprintf(text, sizeof(text),
                "candidate_result sequence=%llu person=%d old_line=%d old_stops=%d,%d destination=%d arrival_bits=%016llx vehicle=%d proposed_line=%d proposed_stops=%d,%d reason=%s board=%d,%d alight=%d,%d places_left=%u requires_engine_revalidation=%u\n",
                (unsigned long long)sequence, proposal.expected.person, proposal.expected.line,
                proposal.expected.boarding_stop, proposal.expected.alighting_stop, proposal.expected.destination,
                (unsigned long long)proposal.expected.arrival_time_bits, v.vehicle, v.line,
                proposal.boarding_stop, proposal.alighting_stop, it_reason_name(proposal.reason),
                proposal.boarding_station, proposal.boarding_terminal, proposal.alighting_station,
                proposal.alighting_terminal, proposal.observed_places_left, proposal.requires_engine_revalidation);
            log_line(text); ++logged;
        }
        return; /* At most one ready vehicle / two validations per callback. */
    }
}
static void sample(tfx_context context, void *unused) {
    (void)unused;
    tfx_observation observation = {.struct_size = sizeof(observation)};
    if (api.v4.v3.v2.base.read_observation(context, &observation) != TFX_OK) return;
    if (failures < 8) {
        discover(context, observation.dispatch_sequence);
        if (failures < 8) evaluate(context, observation.dispatch_sequence);
    }
    if (observation.dispatch_sequence == 1200) {
        char text[256];
        snprintf(text, sizeof(text), "candidate_summary people=%u vehicles=%u ready_samples=%u attempts=%u accepted=%u query_errors=%u\n",
            person_count, vehicle_count, ready, attempts, accepted, failures);
        log_line(text);
        for (unsigned i = 0; i < IT_REASON_COUNT; ++i) if (reasons[i]) {
            snprintf(text, sizeof(text), "candidate_reason name=%s count=%u\n", it_reason_name((it_reason)i), reasons[i]);
            log_line(text);
        }
    }
}
tfx_status tfx_register_candidate_probe(void (*write_line)(const char *)) {
    log_line = write_line;
    tfx_status status = tfx_get_api(TFX_TERMINAL_ABI_VERSION, &api.v4.v3.v2.base);
    if (status != TFX_OK) return status;
    tfx_client client = {.struct_size = sizeof(client), .id = "research.interlining-candidates",
        .before_terminal_selection = sample};
    return api.v4.v3.v2.base.register_client(&client);
}
