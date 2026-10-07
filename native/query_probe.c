/* Bounded research consumer. Every read goes through the public API. */
#include <stdio.h>
#include <string.h>
#include "query_probe.h"

static tfx_api_v4 api = {.v3.v2.base.struct_size = sizeof(api)};
static void (*log_line)(const char *);
static int32_t cursor, people[32], vehicles[16], previous_assignment[32];
static uint32_t person_count, vehicle_count, person_cursor, vehicle_cursor;
static uint32_t failures, platform_logs, vehicle_logs, reservation_logs;
static tfx_person_platforms last_platforms[32];
static tfx_vehicle last_vehicles[16];

static void failure(const char *query, int32_t entity, tfx_status status) {
    if (failures++ >= 8) return;
    char text[160];
    snprintf(text, sizeof(text), "query_error operation=%s entity=%d status=%u\n", query, entity, status);
    log_line(text);
}

static void discover(tfx_context context) {
    tfx_waiting_page page = {.struct_size = sizeof(page)};
    tfx_status status = api.v3.v2.scan_waiting_people(context, cursor, &page);
    if (status != TFX_OK) { failure("scan", cursor, status); return; }
    cursor = page.next_entity < 0 ? 0 : page.next_entity;
    for (uint32_t i = 0; i < page.count && person_count < 32; ++i) {
        uint32_t at = 0;
        while (at < person_count && people[at] != page.persons[i]) ++at;
        if (at != person_count) continue;
        people[person_count] = page.persons[i];
        previous_assignment[person_count++] = -1;
    }
    /* Save-specific IDs belong only to this research consumer, not the SDK. */
    static const int32_t lines[] = {98278, 97742, 97743};
    static uint32_t tick;
    int32_t line_id = lines[tick++ % 3];
    tfx_line_stop stop = {.struct_size = sizeof(stop)};
    status = api.v3.v2.read_line_stop(context, line_id, 0, &stop);
    if (status != TFX_OK) { failure("line", line_id, status); return; }
    tfx_stop_vehicles list = {.struct_size = sizeof(list)};
    int32_t stop_index = (int32_t)((tick / 3) % (uint32_t)stop.stop_count);
    status = api.v3.read_stop_vehicles(context, line_id, stop_index, &list);
    if (status != TFX_OK) { failure("stop_vehicles", line_id, status); return; }
    for (uint32_t i = 0; i < list.count && vehicle_count < 16; ++i) {
        uint32_t at = 0;
        while (at < vehicle_count && vehicles[at] != list.vehicles[i]) ++at;
        if (at == vehicle_count) vehicles[vehicle_count++] = list.vehicles[i];
    }
}

static int has_platform(const tfx_person_platforms *p, const tfx_line_stop *s) {
    for (uint32_t i = 0; i < p->count; ++i)
        if (p->platforms[i].station_entity == s->station_entity && p->platforms[i].terminal == s->terminal)
            return 1;
    return 0;
}

static void observe_people(tfx_context context, uint64_t sequence) {
    static uint32_t platform_tick;
    uint32_t count = person_count < 8 ? person_count : 8;
    uint32_t platform_start = count ? platform_tick++ % count : 0;
    for (uint32_t i = 0; i < count && failures < 8; ++i) {
        uint32_t at = person_cursor++ % person_count;
        int32_t person = people[at];
        tfx_waiting_leg leg = {.struct_size = sizeof(leg)};
        tfx_status status = api.v3.v2.read_waiting_leg(context, person, &leg);
        if (status == TFX_NOT_FOUND) continue; /* Passenger may have boarded. */
        if (status != TFX_OK) { failure("waiting", person, status); continue; }
        char text[512];
        if (leg.vehicle >= 0 && previous_assignment[at] != leg.vehicle && reservation_logs < 24) {
            tfx_vehicle v = {.struct_size = sizeof(v)};
            tfx_status vehicle_status = api.v3.read_vehicle(context, leg.vehicle, &v);
            snprintf(text, sizeof(text),
                "query_assignment sequence=%llu person=%d line=%d stops=%d,%d vehicle=%d vehicle_status=%u capacity=%u onboard=%u reserved=%u places_left=%u reservation_present=%u\n",
                (unsigned long long)sequence, person, leg.line, leg.boarding_stop, leg.alighting_stop,
                leg.vehicle, vehicle_status, v.passenger_capacity, v.passengers_onboard, v.passengers_reserved,
                v.passenger_places_left, vehicle_status == TFX_OK && v.passengers_reserved > 0);
            log_line(text); ++reservation_logs;
        }
        previous_assignment[at] = leg.vehicle;
        /* Two platform lookups per callback keep map scans within shared budgets. */
        if ((i + count - platform_start) % count >= 2 || platform_logs >= 48 ||
            !(leg.flags & TFX_HAS_TERMINAL_PLACE)) continue;
        tfx_person_platforms p = {.struct_size = sizeof(p)};
        status = api.read_person_platforms(context, person, &p);
        if (status != TFX_OK) { failure("platforms", person, status); continue; }
        if (!memcmp(&last_platforms[at], &p, sizeof(p))) continue; /* Logging only; never eligibility cache. */
        last_platforms[at] = p;
        tfx_line_stop board = {.struct_size = sizeof(board)};
        tfx_status board_status = api.v3.v2.read_line_stop(context, leg.line, leg.boarding_stop, &board);
        snprintf(text, sizeof(text),
            "query_platforms sequence=%llu person=%d line=%d stop=%d edge=%d,%d place=%d count=%u board_status=%u configured_member=%u\n",
            (unsigned long long)sequence, person, leg.line, leg.boarding_stop, p.edge_entity, p.edge_index,
            p.place, p.count, board_status, board_status == TFX_OK && has_platform(&p, &board));
        log_line(text);
        for (uint32_t j = 0; j < p.count; ++j) {
            snprintf(text, sizeof(text), "query_platform person=%d station=%d terminal=%d\n",
                person, p.platforms[j].station_entity, p.platforms[j].terminal);
            log_line(text);
        }
        /* Compare physical membership at shared Wimborne, without declaring route eligibility. */
        if (board_status == TFX_OK && board.station_group == 97291) {
            const int32_t lines[] = {98278, 97742, 97743};
            for (unsigned j = 0; j < 3; ++j) {
                tfx_line_stop other = {.struct_size = sizeof(other)};
                status = api.v3.v2.read_line_stop(context, lines[j], 1, &other);
                snprintf(text, sizeof(text), "query_shared_platform person=%d line=%d status=%u member=%u\n",
                    person, lines[j], status, status == TFX_OK && has_platform(&p, &other));
                log_line(text);
            }
        }
        ++platform_logs;
    }
}

static void observe_vehicles(tfx_context context, uint64_t sequence) {
    uint32_t count = vehicle_count < 4 ? vehicle_count : 4;
    for (uint32_t i = 0; i < count && failures < 8; ++i) {
        uint32_t at = vehicle_cursor++ % vehicle_count;
        tfx_vehicle v = {.struct_size = sizeof(v)};
        tfx_status status = api.v3.read_vehicle(context, vehicles[at], &v);
        if (status == TFX_NOT_FOUND) continue;
        if (status != TFX_OK) { failure("vehicle", vehicles[at], status); continue; }
        tfx_vehicle previous = last_vehicles[at];
        last_vehicles[at] = v;
        previous.next_passenger_load_seconds_bits = v.next_passenger_load_seconds_bits;
        if (!memcmp(&previous, &v, sizeof(v))) continue;
        if (vehicle_logs >= 64 && (!v.passengers_reserved || reservation_logs >= 24)) continue;
        char text[512];
        snprintf(text, sizeof(text),
            "query_vehicle sequence=%llu vehicle=%d line=%d stop=%d state=%d load_state=%d station_index=%d terminal=%d locked=%u capacity=%u onboard=%u reserved=%u places_left=%u\n",
            (unsigned long long)sequence, v.vehicle, v.line, v.stop_index, v.state, v.load_state,
            v.arrival_station_index, v.arrival_terminal, v.arrival_locked, v.passenger_capacity,
            v.passengers_onboard, v.passengers_reserved, v.passenger_places_left);
        log_line(text); ++vehicle_logs;
        if (v.passengers_reserved) ++reservation_logs;
    }
}

static void sample(tfx_context context, void *unused) {
    (void)unused;
    if (failures >= 8) return;
    tfx_observation observation = {.struct_size = sizeof(observation)};
    if (api.v3.v2.base.read_observation(context, &observation) != TFX_OK) return;
    discover(context);
    observe_people(context, observation.dispatch_sequence);
    observe_vehicles(context, observation.dispatch_sequence);
}

tfx_status tfx_register_query_probe(void (*write_line)(const char *)) {
    log_line = write_line;
    tfx_status status = tfx_get_api(TFX_PLATFORM_ABI_VERSION, &api.v3.v2.base);
    if (status != TFX_OK) return status;
    tfx_client client = {.struct_size = sizeof(client), .id = "research.passenger-route-observer",
        .before_terminal_selection = sample};
    return api.v3.v2.base.register_client(&client);
}
